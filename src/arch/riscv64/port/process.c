/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Process for RISCV64
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-16      JasonHu           Init
 */

#include <sched/process.h>
#include <sched/syscall.h>
#include <mm/alloc.h>
#include <utils/memory.h>
#include <mm/page.h>
#define NX_LOG_NAME "syscall"
#define NX_LOG_LEVEL NX_LOG_INFO
#include <utils/log.h>
#include <xbook/debug.h>
#include <platform.h>
#include <mm/mmu.h>
#include <interrupt.h>

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
    NX_Addr pageTablePhy = (NX_Addr)NX_Virt2Phy(pageTableVir);
    /* no need switch same page table */
    if (pageTablePhy != NX_MmuGetPageTable())
    {
        NX_MmuSetPageTable(pageTablePhy);
    }
    return NX_EOK;
}

NX_PUBLIC void HAL_ProcessSyscallDispatch(HAL_TrapFrame *frame)
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

NX_IMPORT void HAL_ProcessEnterUserMode(void *args, const void *text, void *userStack);
NX_PRIVATE void HAL_ProcessExecuteUser(const void *text, void *userStack, void *kernelStack, void *args)
{
    NX_LOG_D("riscv64 process enter user: %p, user stack %p", text, userStack);
    HAL_ProcessEnterUserMode(args, text, userStack);
    NX_PANIC("should never return after into user");
}

NX_PRIVATE void *HAL_ProcessGetKernelPageTable(void)
{
    return HAL_GetKernelPageTable();
}

NX_INTERFACE struct NX_ProcessOps NX_ProcessOpsInterface = 
{
    .initUserSpace      = HAL_ProcessInitUserSpace,
    .switchPageTable    = HAL_ProcessSwitchPageTable,
    .getKernelPageTable = HAL_ProcessGetKernelPageTable,
    .executeUser        = HAL_ProcessExecuteUser,
    .freePageTable      = HAL_ProcessFreePageTable,
};
