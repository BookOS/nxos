/**
 * Copyright (c) 2018-2022, BookOS Development Team
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
#define NX_LOG_NAME "Core"
#include <utils/log.h>

#include <xbook/debug.h>

NX_IMPORT NX_Error NX_HalInitClock(void);

NX_STATIC_ATOMIC_INIT(NX_ActivedCoreCount, 0);

/* init as zero, avoid cleared by clear bss action */
NX_PRIVATE NX_VOLATILE NX_UArch BootCoreId = 0;

NX_PRIVATE NX_Cpu CpuArray[NX_MULTI_CORES_NR];

void NX_SMP_Preload(NX_UArch coreId)
{
    /* recored boot core */
    BootCoreId = coreId;
}

NX_UArch NX_SMP_GetBootCore(void)
{
    return BootCoreId;
}

/**
 * init multi core before thread
 */
void NX_SMP_Init(NX_UArch coreId)
{
    /* init core array */
    int i;
    for (i = 0; i < NX_MULTI_CORES_NR; i++)
    {
        CpuArray[i].threadRunning = NX_NULL;
        NX_ListInit(&CpuArray[i].threadReadyList);
        NX_SpinInit(&CpuArray[i].lock);
        NX_AtomicSet(&CpuArray[i].threadCount, 0);
    }
}

/**
 * get CPU by index
 */
NX_Cpu *NX_CpuGetIndex(NX_UArch coreId)
{
    NX_ASSERT(coreId < NX_MULTI_CORES_NR);
    return &CpuArray[coreId];
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
    NX_Cpu *cpu = NX_CpuGetIndex(coreId);

    NX_SpinLock(&cpu->lock, NX_True);

    if (flags & NX_SCHED_HEAD)
    {
        NX_ListAdd(&thread->list, &cpu->threadReadyList);
    }
    else
    {
        NX_ListAddTail(&thread->list, &cpu->threadReadyList);
    }

    NX_AtomicInc(&cpu->threadCount);

    NX_SpinUnlock(&cpu->lock);
}

NX_Thread *NX_SMP_DeququeThreadIrqDisabled(NX_UArch coreId)
{
    NX_Thread *thread;
    NX_Cpu *cpu = NX_CpuGetIndex(coreId);
    
    NX_SpinLock(&cpu->lock, NX_True);
    
    thread = NX_ListFirstEntry(&cpu->threadReadyList, NX_Thread, list);
    NX_ListDel(&thread->list);

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
    
    NX_SpinLock(&cpu->lock, NX_True);
    
    NX_ListForEachEntry(thread, &cpu->threadReadyList, list)
    {
        if (thread->coreAffinity >= NX_MULTI_CORES_NR) /* no affinity on any core */
        {
            findThread = thread;
            NX_ListDel(&thread->list);
            NX_AtomicDec(&cpu->threadCount);
            break;
        }
    }
    
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
