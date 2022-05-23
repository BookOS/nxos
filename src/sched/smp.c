/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: mutil core sched
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-10     JasonHu           Init
 */

#include <sched/smp.h>
#include <sched/thread.h>
#include <sched/sched.h>
#include <io/irq.h>
#define NX_LOG_NAME "smp"
#define NX_LOG_LEVEL NX_LOG_INFO
#include <utils/log.h>

#include <xbook/debug.h>

NX_IMPORT NX_Error NX_HalInitClock(void);

NX_ATOMIC_DEFINE(gActivedCoreCount, 0);

/* init as zero, avoid cleared by clear bss action */
NX_PRIVATE NX_VOLATILE NX_UArch bootCoreId = 0;

NX_PRIVATE NX_Cpu cpuArray[NX_MULTI_CORES_NR];

void NX_SMP_Preload(NX_UArch coreId)
{
    /* recored boot core */
    bootCoreId = coreId;
}

NX_UArch NX_SMP_GetBootCore(void)
{
    return bootCoreId;
}

/**
 * init multi core before thread
 */
void NX_SMP_Init(NX_UArch coreId)
{
    /* init core array */
    int i, j;
    for (i = 0; i < NX_MULTI_CORES_NR; i++)
    {
        cpuArray[i].threadRunning = NX_NULL;
        cpuArray[i].idleThread = NX_NULL;
        cpuArray[i].idleElapsedTicks = 0;
        cpuArray[i].idleTime = 0;
        for (j = 0; j < NX_THREAD_MAX_PRIORITY_NR; j++)
        {
            NX_ListInit(&cpuArray[i].threadReadyList[j]);
        }
        NX_SpinInit(&cpuArray[i].lock);
        NX_AtomicSet(&cpuArray[i].threadCount, 0);
    }
}

/**
 * get CPU by index
 */
NX_Cpu *NX_CpuGetIndex(NX_UArch coreId)
{
    NX_ASSERT(coreId < NX_MULTI_CORES_NR);
    return &cpuArray[coreId];
}

void NX_SMP_Main(NX_UArch coreId)
{
    /* boot other cores */
    if (NX_SMP_BootApp(coreId) != NX_EOK)
    {
        NX_LOG_W("Boot multi core failed!");
    }
    else
    {
        NX_LOG_I("Boot multi core success!");
    }
}

void NX_SMP_Stage2(NX_UArch appCoreId)
{
    NX_Error err;
    err = NX_SMP_EnterApp(appCoreId);
    if (err != NX_EOK)
    {
        NX_LOG_E("app core: %d setup failed!", appCoreId);
    }
    else
    {
        if (NX_HalInitClock() != NX_EOK)
        {
            NX_LOG_E("app core: %d init clock failed!", appCoreId);
        }
        else
        {
            NX_LOG_I("app core: %d setup success!", appCoreId);    
        }
    }
}

void NX_SMP_EnqueueThreadIrqDisabled(NX_UArch coreId, NX_Thread *thread, int flags)
{
    NX_ASSERT(thread->priority >= 0 && thread->priority < NX_THREAD_MAX_PRIORITY_NR);

    NX_Cpu *cpu = NX_CpuGetIndex(coreId);

    NX_SpinLock(&cpu->lock);

    if (flags & NX_SCHED_HEAD)
    {
        NX_ListAdd(&thread->list, &cpu->threadReadyList[thread->priority]);
    }
    else
    {
        NX_ListAddTail(&thread->list, &cpu->threadReadyList[thread->priority]);
    }

    NX_AtomicInc(&cpu->threadCount);

    NX_SpinUnlock(&cpu->lock);
}

void NX_SMP_DequeueThreadIrqDisabled(NX_UArch coreId, NX_Thread *thread)
{
    NX_ASSERT(thread->priority >= 0 && thread->priority < NX_THREAD_MAX_PRIORITY_NR);

    NX_Cpu *cpu = NX_CpuGetIndex(coreId);

    NX_SpinLock(&cpu->lock);

    NX_ListDel(&thread->list);
    NX_AtomicDec(&cpu->threadCount);

    NX_SpinUnlock(&cpu->lock);
}

void NX_SMP_DequeueThread(NX_UArch coreId, NX_Thread *thread)
{
    NX_UArch level;
    level = NX_IRQ_SaveLevel();

    NX_SMP_DequeueThreadIrqDisabled(thread->onCore, thread);
    
    NX_IRQ_RestoreLevel(level);
}

/**
 * This is based on a multi-level feedback queue scheduling algorithm to do the calculations.
 */
NX_PRIVATE void NX_ThreadLowerPriority(NX_Thread *thread)
{
    /* Time-sharing scheduling requires lowering the priority of threads */
    if (thread->fixedPriority >= NX_THREAD_PRIORITY_LOW && thread->fixedPriority <= NX_THREAD_PRIORITY_HIGH)
    {
        --thread->priority;
        if (thread->priority < NX_THREAD_PRIORITY_LOW)
        {
            thread->priority = thread->fixedPriority; /* rebase priority */
        }
    }
}

NX_Thread *NX_SMP_PickThreadIrqDisabled(NX_UArch coreId)
{
    NX_Thread *thread = NX_NULL;
    int prio;
    NX_Cpu *cpu = NX_CpuGetIndex(coreId);
    
    NX_SpinLock(&cpu->lock);
    
    for (prio = NX_THREAD_MAX_PRIORITY_NR - 1; prio >= 0; prio--)
    {
        thread = NX_ListFirstEntryOrNULL(&cpu->threadReadyList[prio], NX_Thread, list);
        if (thread != NX_NULL)
        {
            break;
        }
    }

    /* at least one idle thread on core */
    NX_ASSERT(thread);

    NX_ListDel(&thread->list);

    NX_ThreadLowerPriority(thread);

    NX_AtomicDec(&cpu->threadCount);

    NX_SpinUnlock(&cpu->lock);

    return thread;
}

/**
 * NOTE: this must called irq disabled
 */
NX_Thread *NX_SMP_DeququeNoAffinityThread(NX_UArch coreId)
{
    NX_Thread *thread, *findThread = NX_NULL;
    NX_Cpu *cpu = NX_CpuGetIndex(coreId);
    int prio;

    NX_SpinLock(&cpu->lock);
    
    for (prio = NX_THREAD_MAX_PRIORITY_NR - 1; prio >= 0; prio--)
    {
        NX_ListForEachEntry(thread, &cpu->threadReadyList[prio], list)
        {
            if (thread->coreAffinity >= NX_MULTI_CORES_NR) /* no affinity on any core */
            {
                findThread = thread;
                NX_ListDel(&thread->list);
                NX_AtomicDec(&cpu->threadCount);
                goto out;
            }
        }
    }

out:

    NX_SpinUnlock(&cpu->lock);

    return findThread;
}

NX_Error NX_SMP_SetRunning(NX_UArch coreId, NX_Thread *thread)
{
    if (coreId >= NX_MULTI_CORES_NR || thread == NX_NULL)
    {
        return NX_EINVAL;
    }

    NX_Cpu *cpu = NX_CpuGetIndex(coreId);
    NX_UArch level;
    NX_SpinLockIRQ(&cpu->lock, &level);
    thread->state = NX_THREAD_RUNNING;
    cpu->threadRunning = thread;
    NX_SpinUnlockIRQ(&cpu->lock, level);
    return NX_EOK;
}

NX_Error NX_SMP_SetIdle(NX_UArch coreId, NX_Thread *thread)
{
    if (coreId >= NX_MULTI_CORES_NR || thread == NX_NULL)
    {
        return NX_EINVAL;
    }

    NX_Cpu *cpu = NX_CpuGetIndex(coreId);
    NX_UArch level;
    NX_SpinLockIRQ(&cpu->lock, &level);
    cpu->idleThread = thread;
    NX_SpinUnlockIRQ(&cpu->lock, level);
    return NX_EOK;
}

NX_Thread * NX_SMP_GetIdle(NX_UArch coreId)
{
    NX_Thread *thread;

    if (coreId >= NX_MULTI_CORES_NR)
    {
        return NX_NULL;
    }

    NX_Cpu *cpu = NX_CpuGetIndex(coreId);
    NX_UArch level;
    NX_SpinLockIRQ(&cpu->lock, &level);
    thread = cpu->idleThread;
    NX_SpinUnlockIRQ(&cpu->lock, level);
    return thread;
}

/**
 * get running thread
 */
NX_Thread *NX_SMP_GetRunning(void)
{
    NX_Thread *thread;
    NX_Cpu *cpu = NX_CpuGetPtr();
    NX_ASSERT(cpu != NX_NULL);
    NX_UArch level;
    NX_SpinLockIRQ(&cpu->lock, &level);
    thread = cpu->threadRunning;
    NX_SpinUnlockIRQ(&cpu->lock, level);
    return thread;
}

/**
 * get cpu usage
 */
NX_U32 NX_SMP_GetUsage(NX_UArch coreId)
{
    NX_U32 usage;

    if (coreId >= NX_MULTI_CORES_NR)
    {
        return 0;
    }

    NX_Cpu *cpu = NX_CpuGetIndex(coreId);
    NX_UArch level;
    NX_SpinLockIRQ(&cpu->lock, &level);
    usage = (1000UL - cpu->idleTime) / 10;
    NX_SpinUnlockIRQ(&cpu->lock, level);
    return usage;
}
