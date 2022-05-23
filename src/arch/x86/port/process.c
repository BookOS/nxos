/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Process for x86 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-8       JasonHu           Init
 */

#include <base/process.h>
#include <arch/process.h>
#include <base/malloc.h>
#include <base/memory.h>
#include <base/page.h>
#include <base/mmu.h>
#define NX_LOG_LEVEL NX_LOG_INFO
#include <base/log.h>
#include <base/debug.h>
#include <platform.h>
#include <interrupt.h>
#include <base/syscall.h>
#include <base/thread.h>
#include <segment.h>
#include <regs.h>
#include <tss.h>

NX_IMPORT void NX_HalProcessEnterUserMode(NX_HalTrapFrame *frame);

NX_PRIVATE NX_Error NX_HalProcessInitUserSpace(NX_Process *process, NX_Addr virStart, NX_Size size)
{
    void *table = NX_MemAlloc(NX_PAGE_SIZE);
    if (table == NX_NULL)
    {
        return NX_ENOMEM;
    }
    NX_MemZero(table, NX_PAGE_SIZE);
    NX_MemCopy(table, NX_HalGetKernelPageTable(), NX_PAGE_SIZE);
    NX_MmuInit(&process->vmspace.mmu, table, virStart, size, 0);
    return NX_EOK;
}

NX_PRIVATE NX_Error NX_HalProcessFreePageTable(NX_Vmspace *vmspace)
{
    NX_ASSERT(vmspace);
    if(vmspace->mmu.table == NX_NULL)
    {
        return NX_EFAULT;
    }
    NX_MemFree(vmspace->mmu.table);
    return NX_EOK;
}

NX_PRIVATE NX_Error NX_HalProcessSwitchPageTable(void *pageTableVir)
{
    NX_Thread *cur = NX_ThreadSelf();
    /* need set new kernel esp as next thread */
    CPU_SetTssStack((NX_UArch)(cur->stackBase + cur->stackSize));

    NX_Addr pageTablePhy = (NX_Addr)NX_Virt2Phy(pageTableVir);
    NX_MmuSetPageTable(pageTablePhy);
    return NX_EOK;
}

NX_PRIVATE void *NX_HalProcessGetKernelPageTable(void)
{
    return NX_HalGetKernelPageTable();
}

void NX_HalProcessSyscallDispatch(NX_HalTrapFrame *frame)
{
    NX_SyscallWithArgHandler handler = (NX_SyscallWithArgHandler)NX_SyscallGetHandler((NX_SyscallApi)frame->eax);
    NX_ASSERT(handler);

    /* arg6 saved on the stack */
    NX_U32 arg6 = *(NX_U32 *)(frame->esp);

    NX_LOG_D("x86 syscall api: %x, arg0:%x, arg1:%x, arg2:%x, arg3:%x, arg4:%x, arg5:%x, arg6:%x",
        frame->eax, frame->ebx, frame->ecx, frame->edx, frame->esi, frame->edi, frame->ebp, arg6);

    frame->eax = handler(frame->ebx, frame->ecx, frame->edx, frame->esi, frame->edi, frame->ebp, arg6);

    NX_LOG_D("x86 syscall return: %x", frame->eax);
}

NX_PRIVATE void MakeUserTrapFrame(NX_HalTrapFrame *frame)
{
    frame->ds = frame->es = USER_DATA_SEL;
    frame->cs = USER_CODE_SEL;
    frame->ss = USER_STACK_SEL;
    frame->gs = USER_TLS_SEL;
    frame->fs = 0;  /* unused */

    frame->edi = frame->esi = \
    frame->ebp = frame->espDummy = 0;
    frame->eax = frame->ebx = \
    frame->ecx = frame->edx = 0;

    frame->errorCode = 0;
    frame->vectorNumber = 0;

    /* enable interrupt, user can't use io operation */
    frame->eflags = (EFLAGS_MBS | EFLAGS_IF_1 | EFLAGS_IOPL_3);
}

/**
 * in x86, we can set stack, arg, text entry in a stack frame,
 * then pop them into register, final use iret to switch kernel mode to user mode.
 */
NX_PRIVATE void NX_HalProcessExecuteUser(const void *text, void *userStack, void *kernelStack, void *args)
{
    NX_U8 *stk = kernelStack;
    stk -= sizeof(NX_HalTrapFrame);
    NX_HalTrapFrame *frame = (NX_HalTrapFrame *)stk;

    MakeUserTrapFrame(frame);
    
    frame->esp = (NX_U32)userStack; /* set user stack */
    frame->esp -= sizeof(NX_U32);
    *(NX_U32 *)frame->esp = (NX_U32)args; /* push args into stack */

    frame->eip = (NX_U32)text; /* set user entry */
    NX_HalProcessEnterUserMode(frame);
    NX_PANIC("should never return after into user");
}

NX_IMPORT void __UserThreadReturnCodeBegin();
NX_IMPORT void __UserThreadReturnCodeEnd();

/**
 * in x86, we can set stack, arg, text entry in a stack frame,
 * then pop them into register, final use iret to switch kernel mode to user mode.
 */
NX_PRIVATE void NX_HalProcessExecuteUserThread(const void *text, void *userStack, void *kernelStack, void *arg)
{
    NX_Size retCodeSz;
    NX_U8 * retCode;
    NX_U32 * retStack;
    NX_U8 *stk = kernelStack;
    stk -= sizeof(NX_HalTrapFrame);
    NX_HalTrapFrame *frame = (NX_HalTrapFrame *)stk;

    MakeUserTrapFrame(frame);
    
    /* copy return code */
    retCodeSz = (NX_Size)__UserThreadReturnCodeEnd - (NX_Size)__UserThreadReturnCodeBegin;
    retCode = userStack - retCodeSz;
    NX_MemCopy(retCode, (void *)(NX_Addr)__UserThreadReturnCodeBegin, retCodeSz);

    retStack = (NX_U32 *)NX_ALIGN_DOWN((NX_U32)retCode, sizeof(NX_U32));

    *(--retStack) = (NX_U32) arg;       /* arg */
    *(--retStack) = (NX_U32) retCode;   /* ret eip */

    frame->esp = (NX_U32)retStack;
    frame->eip = (NX_U32)text;
    NX_HalProcessEnterUserMode(frame);
    NX_PANIC("should never return after into user");
}

NX_PRIVATE void NX_HalProcessSetTls(void *tls)
{
    CPU_TlsSet((NX_Addr)tls);
}

NX_PRIVATE void * NX_HalProcessGetTls(void)
{
    return (void *)CPU_TlsGet();
}

NX_INTERFACE struct NX_ProcessOps NX_ProcessOpsInterface = 
{
    .initUserSpace      = NX_HalProcessInitUserSpace,
    .switchPageTable    = NX_HalProcessSwitchPageTable,
    .getKernelPageTable = NX_HalProcessGetKernelPageTable,
    .executeUser        = NX_HalProcessExecuteUser,
    .executeUserThread  = NX_HalProcessExecuteUserThread,
    .freePageTable      = NX_HalProcessFreePageTable,
    .setTls             = NX_HalProcessSetTls,
    .getTls             = NX_HalProcessGetTls,
};
