/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: utest for semaphore test 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-12      JasonHu           Init
 */

#include <base/semaphore.h>
#include <test/utest.h>

#ifdef CONFIG_NX_UTEST_SCHED_SEMAPHORE

NX_TEST(NX_SemaphoreInit)
{
    NX_Semaphore sem;
    NX_EXPECT_NE(NX_SemaphoreInit(NX_NULL, 0), NX_EOK);
    NX_EXPECT_EQ(NX_SemaphoreInit(&sem, 0), NX_EOK);
    NX_EXPECT_EQ(NX_SemaphoreInit(&sem, 1), NX_EOK);
}

NX_TEST(NX_SemaphoreWait)
{
    NX_Semaphore sem;
    NX_Semaphore sem2;

    NX_EXPECT_EQ(NX_SemaphoreInit(&sem, 3), NX_EOK);

    NX_EXPECT_NE(NX_SemaphoreWait(NX_NULL), NX_EOK);
    NX_EXPECT_NE(NX_SemaphoreWait(&sem2), NX_EOK);

    NX_EXPECT_EQ(NX_SemaphoreWait(&sem), NX_EOK);
    NX_EXPECT_EQ(NX_SemaphoreWait(&sem), NX_EOK);
    NX_EXPECT_EQ(NX_SemaphoreWait(&sem), NX_EOK);
}

NX_TEST(NX_SemaphoreSignal)
{
    NX_Semaphore sem;
    NX_Semaphore sem2;

    NX_EXPECT_EQ(NX_SemaphoreInit(&sem, 0), NX_EOK);

    NX_EXPECT_NE(NX_SemaphoreSignal(NX_NULL), NX_EOK);
    NX_EXPECT_NE(NX_SemaphoreSignal(&sem2), NX_EOK);

    NX_EXPECT_EQ(NX_SemaphoreSignal(&sem), NX_EOK);
    NX_EXPECT_EQ(NX_SemaphoreWait(&sem), NX_EOK);

    NX_EXPECT_EQ(NX_SemaphoreSignal(&sem), NX_EOK);
    NX_EXPECT_EQ(NX_SemaphoreSignal(&sem), NX_EOK);
    
    NX_EXPECT_EQ(NX_SemaphoreWait(&sem), NX_EOK);
    NX_EXPECT_EQ(NX_SemaphoreWait(&sem), NX_EOK);
}

NX_TEST_TABLE(NX_Semaphore)
{
    NX_TEST_UNIT(NX_SemaphoreInit),
    NX_TEST_UNIT(NX_SemaphoreWait),
    NX_TEST_UNIT(NX_SemaphoreSignal),
};

NX_TEST_CASE(NX_Semaphore);

#endif
