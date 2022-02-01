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
#include <mm/alloc.h>
#include <utils/memory.h>
#include <mm/page.h>
#include <mmu.h>
#define NX_LOG_LEVEL NX_LOG_INFO
#include <utils/log.h>
#include <xbook/debug.h>
#include <platform.h>
#include <interrupt.h>
#include <sched/syscall.h>

NX_PRIVATE NX_Error HAL_ProcessInitUserSpace(NX_Process *process)
{
    process->pageTable = NX_MemAlloc(NX_PAGE_SIZE);
    NX_ASSERT(process->pageTable != NX_NULL);
    NX_MemZero(process->pageTable, NX_PAGE_SIZE);
    NX_MemCopy(process->pageTable, HAL_GetKernelPageTable(), NX_PAGE_SIZE);
    return NX_EOK;
}

NX_PRIVATE NX_Error HAL_ProcessSwitchPageTable(void *pageTableVir)
{
    NX_Addr pageTablePhy = (NX_Addr)NX_Virt2Phy(pageTableVir);
    /* no need switch same page table */
    if (pageTablePhy != MMU_GetPageTable())
    {
        MMU_SetPageTable(pageTablePhy);
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

    NX_LOG_D("x86 syscall api: %x, arg0:%x, arg1:%x, arg2:%x, arg3:%x, arg4:%x, arg5:%x, arg6:%x",
        frame->eax, frame->ebx, frame->ecx, frame->edx, frame->esi, frame->edi, frame->ebp, 0);

    frame->eax = handler(frame->ebx, frame->ecx, frame->edx, frame->esi, frame->edi, frame->ebp, 0);

    NX_LOG_D("x86 syscall return: %x", frame->eax);
}

NX_INTERFACE struct NX_ProcessOps NX_ProcessOpsInterface = 
{
    .initUserSpace      = HAL_ProcessInitUserSpace,
    .switchPageTable    = HAL_ProcessSwitchPageTable,
    .getKernelPageTable = HAL_ProcessGetKernelPageTable,
};
