/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: utest for timer 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-2      JasonHu           Init
 */

#include <test/utest.h>

#include <time/timer.h>

#ifdef CONFIG_NX_UTEST_MODS_TIMER

NX_PRIVATE int timerOneshotFlags = 0;

NX_PRIVATE NX_Bool NX_TimerHandler(NX_Timer *timer, void *arg)
{
    timerOneshotFlags++;
    if (arg == (void *)0x1234abcd)
    {
        timerOneshotFlags++;    
    }
    return NX_True;
}

#define NX_TIMER_PERIOD_COUNT 10
NX_PRIVATE int timerPeriodFlags = 0;
NX_PRIVATE NX_Bool NX_TimerHandler2(NX_Timer *timer, void *arg)
{
    timerPeriodFlags++;
    if (timerPeriodFlags == NX_TIMER_PERIOD_COUNT)
    {
        return NX_False; /* no next timer */
    }
    return NX_True;
}

NX_TEST(TimerCreateAndDestroy)
{
    NX_Timer *timer0 = NX_TimerCreate(1000, NX_TimerHandler, (void *)0x1234abcd, NX_TIMER_ONESHOT);
    NX_EXPECT_NOT_NULL(timer0);

    NX_Timer *timer1 = NX_TimerCreate(1000, NX_TimerHandler, (void *)0x1234abcd, NX_TIMER_ONESHOT);
    NX_EXPECT_NOT_NULL(timer1);

    NX_Timer *timer2 = NX_TimerCreate(0, NX_TimerHandler, (void *)0x1234abcd, NX_TIMER_ONESHOT);
    NX_EXPECT_NULL(timer2);

    NX_Timer *timer3 = NX_TimerCreate(10, NX_NULL, (void *)0x1234abcd, NX_TIMER_ONESHOT);
    NX_EXPECT_NULL(timer3);

    NX_Timer *timer4 = NX_TimerCreate(10, NX_TimerHandler, NX_NULL, NX_TIMER_ONESHOT);
    NX_EXPECT_NOT_NULL(timer4);

    NX_EXPECT_NE(NX_TimerDestroy(NX_NULL), NX_EOK);
    NX_EXPECT_EQ(NX_TimerDestroy(timer0), NX_EOK);
    NX_EXPECT_EQ(NX_TimerDestroy(timer1), NX_EOK);
    NX_EXPECT_NE(NX_TimerDestroy(timer2), NX_EOK);
    NX_EXPECT_NE(NX_TimerDestroy(timer3), NX_EOK);
    NX_EXPECT_EQ(NX_TimerDestroy(timer4), NX_EOK);
}

NX_TEST(TimerStart)
{
    timerOneshotFlags = 0;
    NX_EXPECT_NE(NX_TimerStart(NX_NULL), NX_EOK);
    NX_Timer *timer0 = NX_TimerCreate(100, NX_TimerHandler, (void *)0x1234abcd, NX_TIMER_ONESHOT);
    NX_EXPECT_NOT_NULL(timer0);
    NX_EXPECT_EQ(NX_TimerStart(timer0), NX_EOK);
    NX_ClockTickDelayMillisecond(150);
    NX_EXPECT_EQ(timerOneshotFlags, 2);

    timerPeriodFlags = 0;
    NX_Timer *timer1 = NX_TimerCreate(100, NX_TimerHandler2, (void *)0x1234abcd, NX_TIMER_PERIOD);
    NX_EXPECT_NOT_NULL(timer1);
    NX_EXPECT_EQ(NX_TimerStart(timer1), NX_EOK);
    NX_ClockTickDelayMillisecond(1100);
    NX_EXPECT_EQ(timerPeriodFlags, NX_TIMER_PERIOD_COUNT);
}

NX_PRIVATE NX_Bool NX_TimerStopHandler(NX_Timer *timer, void *arg)
{
    NX_EXPECT_FALSE(1); /* should never occur! */
    return NX_True;
}

NX_PRIVATE int stopTimerOccurTimes = 0;
NX_PRIVATE NX_Bool NX_TimerStopHandler2(NX_Timer *timer, void *arg)
{
    stopTimerOccurTimes++;
    NX_EXPECT_EQ(stopTimerOccurTimes, 1);
    return NX_False; /* no next timer */
}

NX_TEST(TimerStop)
{
    NX_Timer *timer1 = NX_TimerCreate(100, NX_TimerStopHandler, NX_NULL, NX_TIMER_ONESHOT);
    NX_EXPECT_NOT_NULL(timer1);
    NX_EXPECT_EQ(NX_TimerStart(timer1), NX_EOK);
    NX_ClockTickDelayMillisecond(50);
    /* stop timer before occur! */
    NX_EXPECT_EQ(NX_TimerStop(timer1), NX_EOK);
    NX_EXPECT_EQ(NX_TimerDestroy(timer1), NX_EOK);
    
    stopTimerOccurTimes = 0;
    NX_Timer *timer2 = NX_TimerCreate(100, NX_TimerStopHandler2, NX_NULL, NX_TIMER_PERIOD);
    NX_EXPECT_NOT_NULL(timer2);
    NX_EXPECT_EQ(NX_TimerStart(timer2), NX_EOK);
    NX_ClockTickDelayMillisecond(200);
}

NX_TEST_TABLE(NX_Timer)
{
    NX_TEST_UNIT(TimerCreateAndDestroy),
    NX_TEST_UNIT(TimerStart),
    NX_TEST_UNIT(TimerStop),
};

NX_TEST_CASE(NX_Timer);

#endif
