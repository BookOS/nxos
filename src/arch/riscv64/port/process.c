/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Process for RISCV64
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-16      JasonHu           Init
 */

#include <process/process.h>
#include <process/syscall.h>
#include <mm/alloc.h>
#include <utils/memory.h>
#include <mm/page.h>
#define NX_LOG_NAME "syscall"
#define NX_LOG_LEVEL NX_LOG_INFO
#include <utils/log.h>
#include <xbook/debug.h>
#include <platform.h>
#include <mm/mmu.h>
#include <sched/thread.h>
#include <interrupt.h>
#include <regs.h>

NX_PRIVATE void NX_HalProcessSetTls(void *tls);

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
    NX_Addr pageTablePhy = (NX_Addr)NX_Virt2Phy(pageTableVir);
    /* no need switch same page table */
    if (pageTablePhy != NX_MmuGetPageTable())
    {
        NX_MmuSetPageTable(pageTablePhy);
    }
    return NX_EOK;
}

void NX_HalProcessSyscallDispatch(NX_HalTrapFrame *frame)
{
    NX_SyscallWithArgHandler handler = (NX_SyscallWithArgHandler)NX_SyscallGetHandler((NX_SyscallApi)frame->a7);
    NX_ASSERT(handler);

    NX_LOG_D("riscv64 syscall api: %x, arg0:%x, arg1:%x, arg2:%x, arg3:%x, arg4:%x, arg5:%x, arg6:%x",
        frame->a7, frame->a0, frame->a1, frame->a2, frame->a3, frame->a4, frame->a5, frame->a6);

    frame->a0 = handler(frame->a0, frame->a1, frame->a2, frame->a3, frame->a4, frame->a5, frame->a6);
    frame->a7 = 0; /* clear syscall api */
    frame->epc += 4; /* skip ecall instruction */

    NX_LOG_D("riscv64 syscall return: %x", frame->a0);
}

NX_IMPORT void NX_HalProcessEnterUserMode(void *args, const void *text, void *userStack, void * returnAddr);
NX_PRIVATE void NX_HalProcessExecuteUser(const void *text, void *userStack, void *kernelStack, void *args)
{
    NX_Thread * self;

    self = NX_ThreadSelf();

    /* update tls before enter user */
    NX_HalProcessSetTls(self->resource.tls);

    NX_LOG_D("riscv64 process enter user: %p, user stack %p", text, userStack);
    NX_HalProcessEnterUserMode(args, text, userStack, NX_NULL);
    NX_PANIC("should never return after into user");
}

NX_IMPORT void __UserThreadReturnCodeBegin();
NX_IMPORT void __UserThreadReturnCodeEnd();

NX_PRIVATE void NX_HalProcessExecuteUserThread(const void *text, void *userStack, void *kernelStack, void *arg)
{
    NX_Size retCodeSz;
    NX_U8 * retCode;
    NX_Addr * retStack;
    NX_Thread * self;

    self = NX_ThreadSelf();

    /* update tls before enter user */
    NX_HalProcessSetTls(self->resource.tls);

    /* copy return code */
    retCodeSz = (NX_Size)__UserThreadReturnCodeEnd - (NX_Size)__UserThreadReturnCodeBegin;
    retCode = userStack - retCodeSz;
    NX_MemCopy(retCode, (void *)(NX_Addr)__UserThreadReturnCodeBegin, retCodeSz);
    
    retStack = (NX_Addr *)NX_ALIGN_DOWN((NX_Addr)retCode, sizeof(NX_Addr));

    NX_LOG_D("riscv64 process enter user thread: %p, user stack %p", text, retStack);
    NX_HalProcessEnterUserMode(arg, text, retStack, retCode);
    NX_PANIC("should never return after into user");
}

NX_PRIVATE void *NX_HalProcessGetKernelPageTable(void)
{
    return NX_HalGetKernelPageTable();
}

NX_PRIVATE void NX_HalProcessSetTls(void *tls)
{
    WriteReg(tp, (NX_Addr)tls);
}

NX_PRIVATE void * NX_HalProcessGetTls(void)
{
    return (void *)ReadReg(tp);
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
