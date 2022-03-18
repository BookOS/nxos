/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Heap Cache
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-15     JasonHu           Init
 */

#include <test/integration.h>
#include <time/timer.h>
#define NX_LOG_NAME "Timer"
#include <utils/log.h>
#include <sched/thread.h>

#ifdef CONFIG_NX_TEST_INTEGRATION_TIMER

NX_PRIVATE NX_Bool NX_TimerHandler1(NX_Timer *timer, void *arg)
{
    NX_LOG_I("Timer#%s timeout\n", (char *)arg);
    return NX_True;
}

NX_PRIVATE NX_VOLATILE int timerCounter = 0;
NX_PRIVATE NX_Bool NX_TimerHandler2(NX_Timer *timer, void *arg)
{
    NX_LOG_I("Timer#%s timeout\n", (char *)arg);
    timerCounter++;
    NX_LOG_I("timerCounter:%d\n", timerCounter);
    if (timerCounter >= 4)
    {
        timerCounter++;
        return NX_False;
    }
    return NX_True;
}

NX_INTEGRATION_TEST(NX_Timer)
{
    timerCounter = 0;
    NX_Timer *timer1 = NX_TimerCreate(500, NX_TimerHandler1, "1", NX_TIMER_ONESHOT);
    if (timer1 == NX_NULL)
    {
        NX_LOG_E("create timer 1 failed!");
        return NX_ERROR;
    }
    if (NX_TimerStart(timer1) != NX_EOK)
    {
        NX_LOG_E("start timer 1 failed!");
        return NX_ERROR;
    }
    NX_LOG_I("start timer 1 OK!");
    
    NX_Timer *timer2 = NX_TimerCreate(1000, NX_TimerHandler2, "2", NX_TIMER_PERIOD);
    if (timer2 == NX_NULL)
    {
        NX_LOG_E("create timer 2 failed!");
        return NX_ERROR;
    }
    if (NX_TimerStart(timer2) != NX_EOK)
    {
        NX_LOG_E("start timer 2 failed!");
        return NX_ERROR;
    }
    NX_LOG_I("start timer 2 OK!");

    NX_Timer *timer3 = NX_TimerCreate(1000, NX_TimerHandler2, "2", NX_TIMER_PERIOD);
    if (timer3 == NX_NULL)
    {
        NX_LOG_E("create timer 2 failed!");
        return NX_ERROR;
    }
    NX_Error err = NX_TimerDestroy(timer3);
    if (err != NX_EOK)
    {
        NX_LOG_E("destroy timer 3 failed!");
        return NX_ERROR;
    }

    while (1)
    {
        if (timerCounter >= 5)
        {
            NX_LOG_I("timerCounter:%d break", timerCounter);
            break;
        }
    }
    NX_LOG_I("timerCounter:%d", timerCounter);
    return NX_EOK;
}

#endif
