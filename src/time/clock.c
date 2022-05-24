/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Clock time
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-31     JasonHu           Init
 */

#include <base/clock.h>
#include <base/timer.h>
#define NX_LOG_LEVEL NX_LOG_INFO 
#include <base/log.h>
#include <base/debug.h>

#include <base/thread.h>
#include <base/sched.h>
#include <base/smp.h>

#include <base/delay_irq.h>
#include <base/time.h>

#define NX_LOG_NAME "Clock"
#include <base/log.h>

NX_IMPORT NX_Error NX_HalInitClock(void);

/* NOTE: must add NX_VOLATILE here, avoid compiler optimization  */
NX_PRIVATE NX_VOLATILE NX_ClockTick systemClockTicks = 0;

NX_PRIVATE NX_IRQ_DelayWork timerWork;
NX_PRIVATE NX_IRQ_DelayWork schedWork;

NX_ClockTick NX_ClockTickGet(void)
{
    return systemClockTicks;
}

void NX_ClockTickSet(NX_ClockTick tick)
{
    systemClockTicks = tick;
}

void NX_ClockTickGo(void)
{
    /* only boot core change system clock and timer */
    if (NX_SMP_GetBootCore() == NX_SMP_GetIdx())
    {
        systemClockTicks++;
        if (systemClockTicks % NX_TICKS_PER_SECOND == 0)
        {
            NX_TimeGo();
        }

        NX_IRQ_DelayWorkHandle(&timerWork);
    }
#ifdef CONFIG_NX_ENABLE_SCHED
    NX_IRQ_DelayWorkHandle(&schedWork);
#endif
}

NX_Error NX_ClockTickDelay(NX_ClockTick ticks)
{
    NX_ClockTick start = NX_ClockTickGet();
    while (NX_ClockTickGet() - start < ticks)
    {
        /* do nothing to delay */

        /**
         * TODO: add thread state check
         * 
         * if (thread exit flags == 1)
         * {
         *     return NX_ETIMEOUT
         * }
        */
    }
    return NX_EOK; 
}

NX_PRIVATE void NX_TimerIrqHandler(void *arg)
{
    NX_TimerGo();
}

NX_PRIVATE void NX_SchedIrqHandler(void *arg)
{
    NX_Thread *thread = NX_ThreadSelf();
    if (thread->isTerminated != 0) /* check exit */
    {
        NX_ThreadExit(1);
    }

    thread->ticks--;
    thread->elapsedTicks++;
    if (thread->ticks == 0)
    {
        // NX_LOG_I("thread:%s need sched", thread->name);
        thread->needSched = 1; /* mark sched */
    }
    NX_ASSERT(thread->ticks >= 0);
}

NX_Error NX_ClockInit(void)
{
    NX_Error err;
    err = NX_IRQ_DelayWorkInit(&timerWork, NX_TimerIrqHandler, NX_NULL, NX_IRQ_WORK_NOREENTER);
    if (err != NX_EOK)
    {
        goto End;
    }
    err = NX_IRQ_DelayWorkInit(&schedWork, NX_SchedIrqHandler, NX_NULL, NX_IRQ_WORK_NOREENTER);
    if (err != NX_EOK)
    {
        goto End;
    }
    err = NX_IRQ_DelayQueueEnter(NX_IRQ_FAST_QUEUE, &timerWork);
    if (err != NX_EOK)
    {
        goto End;
    }
    err = NX_IRQ_DelayQueueEnter(NX_IRQ_SCHED_QUEUE, &schedWork);
    if (err != NX_EOK)
    {
        NX_IRQ_DelayQueueLeave(NX_IRQ_FAST_QUEUE, &timerWork);
        goto End;
    }
    
    err = NX_HalInitClock();
    if (err != NX_EOK)
    {
        NX_IRQ_DelayQueueLeave(NX_IRQ_FAST_QUEUE, &timerWork);
        NX_IRQ_DelayQueueLeave(NX_IRQ_SCHED_QUEUE, &schedWork);
        goto End;
    }

End:
    return err;
}
