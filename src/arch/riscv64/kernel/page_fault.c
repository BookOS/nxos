/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Page fault
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-06-07     JasonHu           Init
 */

#define NX_LOG_LEVEL NX_LOG_INFO
#define NX_LOG_NAME "page fault"
#include <base/log.h>

#include <interrupt.h>
#include <regs.h>
#include <base/thread.h>
#include <arch/process.h>
#include <base/vmspace.h>
#include <base/page.h>

NX_Error HandleNoPage(NX_Thread * thread, NX_Vmspace * vmspace, NX_Vmnode * vmnode, NX_Addr addr)
{
    void * mapAddr = NX_NULL;
    NX_LOG_D("handle no page: %p", addr);
    if (NX_VmspaceMap(vmspace, addr, NX_PAGE_SIZE, vmnode->attr, vmnode->flags, &mapAddr) != NX_EOK)
    {
        NX_LOG_E("thread name=%s tid=%d page %p no read attr", thread->name, thread->tid);
        NX_SignalSend(thread->tid, NX_SIGNAL_MEMACC, (void *)addr, NX_NULL);
    }
    return NX_EOK;
}

NX_Error NX_HalHandlePageFault(NX_Thread * thread, NX_HalTrapFrame * frame, NX_Addr faultAddr)
{
    NX_Addr addr; /* fault addr */
    NX_Process * process;
    NX_Vmnode * vmnode;
    NX_Vmspace * vmspace;

    addr = faultAddr;

    /* check fault in kernel */
    if ((frame->sstatus & SSTATUS_SPP))
    {
        NX_LOG_E("thread name=%s tid=%d addr=%p", thread->name, thread->tid, addr);
        NX_LOG_E("a memory problem had occured in kernel, please check your code! :(");
        return NX_EFAULT;
    }

    /* access kernel addr fault */
    if (!(addr >= NX_USER_SPACE_VADDR && addr < NX_USER_SPACE_TOP))
    {
        NX_LOG_D("thread name=%s tid=%d addr=%p access none user addr", thread->name, thread->tid, addr);
        NX_SignalSend(thread->tid, NX_SIGNAL_MEMACC, (void *)addr, NX_NULL);
        return NX_EOK;
    }

    if ((process = NX_ThreadGetProcess(thread)) == NX_NULL)
    {
        NX_LOG_E("thread name=%s tid=%d addr=%p no process", thread->name, thread->tid, addr);
        return NX_EPERM;
    }

    vmspace = &process->vmspace;
    /* find in vmspace */
    if ((vmnode = VmspaceFindNodeUnder(vmspace, addr)) == NX_NULL)
    {
        NX_LOG_D("thread name=%s tid=%d addr=%p not in vmspace!", thread->name, thread->tid, addr);
        NX_SignalSend(thread->tid, NX_SIGNAL_MEMACC, (void *)addr, NX_NULL);
        return NX_EOK;
    }

    /* check stack, stack need expand lower addr */
    if (addr >= vmspace->stackStart && addr <= vmspace->stackEnd)
    {
        if (addr < vmspace->stackBottom && (addr + 4096) >= frame->sp) /* fault addr under stack, near 4096 bytes */
        {
            /* expand stack */
            vmspace->stackBottom = addr & NX_PAGE_ADDR_MASK;
            NX_LOG_D("expand stack: user name=%s tid=%d addr=%p user task new stack %p!\n", thread->name, thread->tid, addr, vmspace->stackBottom);
        }
        else
        {
            NX_LOG_D("page fault: user name=%s tid=%d addr=%p user task stack out of range!\n", thread->name, thread->tid, addr);
            NX_SignalSend(thread->tid, NX_SIGNAL_STACKFLOW, (void *)addr, NX_NULL);
            return NX_EOK;
        }
    }

    return HandleNoPage(thread, vmspace, vmnode, addr);
}
