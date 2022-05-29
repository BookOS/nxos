/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: NX_Scheduler for NXOS
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-8      JasonHu           Init
 */

#define NX_LOG_LEVEL NX_LOG_INFO
#include <base/log.h>
#include <base/debug.h>
#include <base/irq.h>

#include <base/sched.h>
#include <base/thread.h>
#include <base/smp.h>
#include <base/context.h>
#include <base/process.h>
#include <base/signal.h>

NX_IMPORT NX_ThreadManager gThreadManagerObject;
NX_IMPORT NX_Atomic gActivedCoreCount;

NX_INLINE void SchedSwithProcess(NX_Thread *thread)
{
    NX_Process *process = thread->resource.process;
    void *pageTable = NX_NULL;

    if (process == NX_NULL)
    {
        pageTable = NX_ProcessGetKernelPageTable();
    }
    else
    {
        pageTable = process->vmspace.mmu.table;
    }

    NX_ASSERT(pageTable != NX_NULL);
    NX_ASSERT(NX_ProcessSwitchPageTable(pageTable) == NX_EOK);
    
    /* update tls */
    if (thread->resource.tls)
    {
        NX_ProcessSetTls(thread->resource.tls);
    }
    else
    {
        NX_ProcessSetTls(NX_NULL);
    }
}

NX_INLINE void SchedToNext(NX_Thread *next)
{
    SchedSwithProcess(next);
    NX_ContextSwitchNext((NX_Addr)&next->stack);
}

NX_INLINE void SchedFromPrevToNext(NX_Thread *prev, NX_Thread *next)
{
    SchedSwithProcess(next);
    NX_ContextSwitchPrevNext((NX_Addr)&prev->stack, (NX_Addr)&next->stack);
}

void NX_SchedToFirstThread(void)
{
    NX_UArch coreId = NX_SMP_GetIdx();
    NX_Thread *thread = NX_SMP_PickThreadIrqDisabled(coreId);
    NX_ASSERT(thread != NX_NULL);
    NX_ASSERT(NX_SMP_SetRunning(coreId, thread) == NX_EOK);
    NX_LOG_D("Sched to first thread:%s/%d", thread->name, thread->tid);
    SchedToNext(thread);
    /* should never be here */
    NX_PANIC("Sched to first thread failed!");
}

NX_PRIVATE void PullOrPushThread(NX_UArch coreId)
{
    NX_Thread *thread;
    NX_Cpu *cpu = NX_CpuGetIndex(coreId);
    NX_IArch coreThreadCount = NX_AtomicGet(&cpu->threadCount);

    /**
     * Adding 1 is to allow the processor core to do more pull operations instead of push operations.
     * Can avoid the threads of the pending queue not running.
     */
    NX_UArch threadsPerCore = NX_AtomicGet(&gThreadManagerObject.activeThreadCount) / NX_AtomicGet(&gActivedCoreCount) + 1;

    NX_LOG_D("core#%d: core threads:%d", coreId, coreThreadCount);
    NX_LOG_D("core#%d: active threads:%d", coreId, NX_AtomicGet(&gThreadManagerObject.activeThreadCount));
    NX_LOG_D("core#%d: pending threads:%d", coreId, NX_AtomicGet(&gThreadManagerObject.pendingThreadCount));
    NX_LOG_D("core#%d: threads per core:%d", coreId, threadsPerCore);

    if (coreThreadCount < threadsPerCore)
    {
        /* pull from pending */
        thread = NX_ThreadPickPendingList();
        if (thread != NX_NULL)
        {
            NX_LOG_D("---> core#%d: pull thread:%s/%d", coreId, thread->name, thread->tid);
            thread->onCore = coreId;
            NX_ThreadReadyRunLocked(thread, NX_SCHED_HEAD);
        }
    }

    if (coreThreadCount > threadsPerCore)
    {
        /* push to pending */
        thread = NX_SMP_DeququeNoAffinityThread(coreId);
        if (thread != NX_NULL)
        {
            NX_LOG_D("---> core#%d: push thread:%s/%d", coreId, thread->name, thread->tid);
            thread->onCore = NX_MULTI_CORES_NR;
            NX_ThreadEnqueuePendingList(thread);
        }
    }
}

/**
 * NOTE: must disable interrupt before call this!
 * @lock: if not NX_NULL, the lock need unblock before sched to other thread
 * @lockIrqLevel: lock irq level
 */
void NX_SchedLockedIRQ(NX_UArch irqLevel, NX_Spin *lock)
{
    NX_Thread *next, *prev;
    NX_UArch coreId = NX_SMP_GetIdx();

    /* put prev into list */
    prev = NX_CurrentThread;

    if (prev->state == NX_THREAD_EXIT)
    {
        prev = NX_NULL;    /* not save prev context */
    }

    /* pull thread from pending list or push thread to pending list */
    PullOrPushThread(coreId);

    /* get next from local list */
    next = NX_SMP_PickThreadIrqDisabled(coreId);
    NX_ASSERT(next != NX_NULL);
    NX_SMP_SetRunning(coreId, next);

    /* unlock lock before sched */
    if (lock)
    {
        NX_SpinUnlock(lock);
    }

    if (prev != NX_NULL)
    {
        NX_ASSERT(prev && next);
        NX_LOG_D("CPU#%d NX_Sched prev: %s/%d next: %s/%d", NX_SMP_GetIdx(), prev->name, prev->tid, next->name, next->tid);
        SchedFromPrevToNext(prev, next);
    }
    else
    {
        SchedToNext(next);
    }
    NX_IRQ_RestoreLevel(irqLevel);
}

/**
 * NOTE: must disable interrupt before call this!
 */
void NX_SchedInterruptDisabled(NX_UArch irqLevel)
{
    NX_SchedLockedIRQ(irqLevel, NX_NULL);
}

void NX_SchedYield(void)
{
    NX_UArch level = NX_IRQ_SaveLevel();

    NX_Thread *cur = NX_CurrentThread;
    
    NX_ThreadReadyRunUnlocked(cur, NX_SCHED_TAIL);

    NX_SchedInterruptDisabled(level);
}

void NX_SchedExit(void)
{
    NX_UArch level = NX_IRQ_SaveLevel();

    NX_Thread *cur = NX_CurrentThread;
    NX_LOG_D("Thread exit: %d", cur->tid);

    cur->state = NX_THREAD_EXIT;
    NX_ThreadEnququeExitList(cur);

    NX_SchedInterruptDisabled(level);
}

void NX_ReSchedCheck(void)
{
    NX_IRQ_Enable();

    NX_Thread *thread = NX_CurrentThread;

    if (thread->isTerminated)
    {
        NX_LOG_D("call terminate: %d", thread->tid);
        thread->isTerminated = 0;
        NX_ThreadExit(1);
    }
    if (thread->needSched)
    {
        NX_UArch level = NX_IRQ_SaveLevel();
        thread->needSched = 0;

        /* reset ticks from timeslice */
        thread->ticks = thread->timeslice;

        NX_ThreadReadyRunUnlocked(thread, NX_SCHED_TAIL);

        NX_SchedInterruptDisabled(level);
    }
    NX_IRQ_Disable();
}
