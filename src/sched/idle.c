/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: idle thread
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-2-18      JasonHu           Init
 */

#include <base/thread.h>
#include <base/debug.h>

#define NX_LOG_NAME "idle"
#include <base/log.h>
#include <base/string.h>
#include <base/timer.h>
#include <base/smp.h>

#define IDLE_TIME_S 1000 /* 1s */

NX_PRIVATE NX_Bool IdleTimer(NX_Timer *tm, void *arg)
{
    NX_UArch coreId = (NX_UArch)arg; 
    NX_Cpu *cpu = NX_CpuGetIndex(coreId);
    NX_TimeVal idleTime;

    NX_ClockTick ticks = cpu->idleThread->elapsedTicks - cpu->idleElapsedTicks;
    cpu->idleElapsedTicks = cpu->idleThread->elapsedTicks;
    idleTime = NX_ClockTickToMillisecond(ticks);
    if (idleTime > IDLE_TIME_S)
    {
        idleTime = IDLE_TIME_S;
    }
    cpu->idleTime = idleTime;
    return NX_True;
}

/**
 * system idle thread on per cpu.
 */
NX_PRIVATE void IdleThreadEntry(void *arg)
{
    NX_Thread *self = NX_ThreadSelf();
    NX_LOG_I("Idle thread: %s startting...", self->name);
    while (1)
    {
        NX_ThreadYield();
    }
}

void NX_ThreadInitIdle(void)
{
    NX_Thread *idleThread;
    NX_UArch coreId;
    char name[8];

    /* init idle thread */
    for (coreId = 0; coreId < NX_MULTI_CORES_NR; coreId++)
    {
        NX_SNPrintf(name, 8, "idle%d", coreId);
        idleThread = NX_ThreadCreate(name, IdleThreadEntry, NX_NULL, NX_THREAD_PRIORITY_IDLE);
        NX_ASSERT(idleThread != NX_NULL);
        /* bind idle on each core */
        NX_ASSERT(NX_ThreadSetAffinity(idleThread, coreId) == NX_EOK);
        NX_ASSERT(NX_ThreadStart(idleThread) == NX_EOK);
        NX_SMP_SetIdle(coreId, idleThread);

        NX_Timer * tmr = NX_TimerCreate(IDLE_TIME_S, IdleTimer, (void *)coreId, NX_TIMER_PERIOD);
        NX_ASSERT(tmr);
        NX_TimerStart(tmr);
    }
}
