/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Process for x86 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-8       JasonHu           Init
 */

#include <sched/process.h>
#include <arch/process.h>
#include <mm/alloc.h>
#include <utils/memory.h>
#include <mm/page.h>
#include <mm/mmu.h>
#define NX_LOG_LEVEL NX_LOG_INFO
#include <utils/log.h>
#include <xbook/debug.h>
#include <platform.h>
#include <interrupt.h>
#include <sched/syscall.h>
#include <sched/thread.h>
#include <segment.h>
#include <regs.h>
#include <tss.h>

NX_IMPORT void HAL_ProcessEnterUserMode(HAL_TrapFrame *frame);

NX_PRIVATE NX_Error HAL_ProcessInitUserSpace(NX_Process *process, NX_Addr virStart, NX_USize size)
{
    void *table = NX_MemAlloc(NX_PAGE_SIZE);
    if (table == NX_NULL)
    {
        return NX_ENOMEM;
    }
    NX_MemZero(table, NX_PAGE_SIZE);
    NX_MemCopy(table, HAL_GetKernelPageTable(), NX_PAGE_SIZE);
    NX_MmuInit(&process->vmspace.mmu, table, virStart, size, 0);
    return NX_EOK;
}

NX_PRIVATE NX_Error HAL_ProcessFreePageTable(NX_Vmspace *vmspace)
{
    NX_ASSERT(vmspace);
    if(vmspace->mmu.table == NX_NULL)
    {
        return NX_EFAULT;
    }
    NX_MemFree(vmspace->mmu.table);
    return NX_EOK;
}

NX_PRIVATE NX_Error HAL_ProcessSwitchPageTable(void *pageTableVir)
{
    NX_Thread *cur = NX_ThreadSelf();
    /* need set new kernel esp as next thread */
    CPU_SetTssStack((NX_UArch)(cur->stackBase + cur->stackSize));

    NX_Addr pageTablePhy = (NX_Addr)NX_Virt2Phy(pageTableVir);
    /* no need switch same page table */
    if (pageTablePhy != NX_MmuGetPageTable())
    {
        NX_MmuSetPageTable(pageTablePhy);
    }
    return NX_EOK;
}

NX_PRIVATE void *HAL_ProcessGetKernelPageTable(void)
{
    return HAL_GetKernelPageTable();
}

NX_PUBLIC void HAL_ProcessSyscallDispatch(HAL_TrapFrame *frame)
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

NX_PRIVATE void MakeUserTrapFrame(HAL_TrapFrame *frame)
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
NX_PRIVATE void HAL_ProcessExecuteUser(const void *text, void *userStack, void *kernelStack, void *args)
{
    NX_U8 *stk = kernelStack;
    stk -= sizeof(HAL_TrapFrame);
    HAL_TrapFrame *frame = (HAL_TrapFrame *)stk;

    MakeUserTrapFrame(frame);
    
    frame->esp = (NX_U32)userStack; /* set user stack */
    frame->esp -= sizeof(NX_U32);
    *(NX_U32 *)frame->esp = (NX_U32)args; /* push args into stack */

    frame->eip = (NX_U32)text; /* set user entry */
    HAL_ProcessEnterUserMode(frame);
    NX_PANIC("should never return after into user");
}

NX_INTERFACE struct NX_ProcessOps NX_ProcessOpsInterface = 
{
    .initUserSpace      = HAL_ProcessInitUserSpace,
    .switchPageTable    = HAL_ProcessSwitchPageTable,
    .getKernelPageTable = HAL_ProcessGetKernelPageTable,
    .executeUser        = HAL_ProcessExecuteUser,
    .freePageTable      = HAL_ProcessFreePageTable,
};
