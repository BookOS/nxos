/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: thread utest 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-27     JasonHu           Init
 */

#include <mods/test/utest.h>
#include <sched/thread.h>

#ifdef CONFIG_NX_UTEST_SCHED_THREAD

NX_TEST(NX_ThreadSleep)
{
    NX_TimeVal s, e;

    NX_EXPECT_EQ(NX_ThreadSleep(1), NX_EOK);
    NX_EXPECT_EQ(NX_ThreadSleep(10), NX_EOK);
    NX_EXPECT_EQ(NX_ThreadSleep(50), NX_EOK);

    s = NX_ClockTickGetMillisecond();
    NX_EXPECT_EQ(NX_ThreadSleep(100), NX_EOK);
    e = NX_ClockTickGetMillisecond();
    NX_EXPECT_GE(e - s, 100);

    s = NX_ClockTickGetMillisecond();
    NX_EXPECT_EQ(NX_ThreadSleep(500), NX_EOK);
    e = NX_ClockTickGetMillisecond();
    NX_EXPECT_GE(e - s, 500);

    s = NX_ClockTickGetMillisecond();
    NX_EXPECT_EQ(NX_ThreadSleep(1000), NX_EOK);
    e = NX_ClockTickGetMillisecond();
    NX_EXPECT_GE(e - s, 100);

    s = NX_ClockTickGetMillisecond();
    NX_EXPECT_EQ(NX_ThreadSleep(3000), NX_EOK);
    e = NX_ClockTickGetMillisecond();
    NX_EXPECT_GE(e - s, 3000);
}

NX_PRIVATE void NX_ThreadSleep1(void *arg)
{
    NX_EXPECT_NE(NX_ThreadSleep(500), NX_EOK);
    /* should never be here */
    NX_ASSERT_TRUE(NX_False);
}

NX_TEST(NX_ThreadSleepIntr)
{
    
    NX_Thread *thread = NX_ThreadCreate("sleep1", NX_ThreadSleep1, NX_NULL);
    NX_EXPECT_NOT_NULL(thread);
    NX_EXPECT_EQ(NX_ThreadRun(thread), NX_EOK);

    NX_EXPECT_EQ(NX_ThreadSleep(100), NX_EOK);

    /* term thread */
    NX_EXPECT_EQ(NX_ThreadTerminate(thread), NX_EOK);

    /* sleep until thread exit */
    NX_EXPECT_EQ(NX_ThreadSleep(1000), NX_EOK);

}

NX_TEST_TABLE(NX_Thread)
{
    NX_TEST_UNIT(NX_ThreadSleep),
    NX_TEST_UNIT(NX_ThreadSleepIntr),
};

NX_TEST_CASE(NX_Thread);

#endif
