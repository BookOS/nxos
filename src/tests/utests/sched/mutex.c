/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: mutex lock test 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-13     JasonHu           Init
 */

#include <sched/mutex.h>
#include <mods/test/utest.h>

#ifdef CONFIG_NX_UTEST_SCHED_MUTEX

NX_TEST(NX_MutexInit)
{
    NX_Mutex lock;
    NX_EXPECT_NE(NX_MutexInit(NX_NULL), NX_EOK);
    NX_EXPECT_EQ(NX_MutexInit(&lock), NX_EOK);
    NX_EXPECT_NE(NX_MutexInit(&lock), NX_EOK);
}

NX_TEST(NX_MutexLock)
{
    NX_Mutex lock;
    NX_Mutex lockNoInit;

    NX_EXPECT_EQ(NX_MutexInit(&lock), NX_EOK);

    NX_EXPECT_NE(NX_MutexLock(NX_NULL, NX_True), NX_EOK);
    NX_EXPECT_NE(NX_MutexLock(&lockNoInit, NX_True), NX_EOK);

    NX_EXPECT_EQ(NX_MutexLock(&lock, NX_True), NX_EOK);
    NX_EXPECT_NE(NX_MutexLock(&lock, NX_False), NX_EOK);
}

NX_TEST(NX_MutexUnlock)
{
    NX_Mutex lock;
    NX_Mutex lockNoInit;
    
    NX_EXPECT_EQ(NX_MutexInit(&lock), NX_EOK);

    NX_EXPECT_NE(NX_MutexUnlock(NX_NULL), NX_EOK);
    NX_EXPECT_NE(NX_MutexUnlock(&lockNoInit), NX_EOK);

    NX_EXPECT_EQ(NX_MutexLock(&lock, NX_True), NX_EOK);
    NX_EXPECT_EQ(NX_MutexUnlock(&lock), NX_EOK);
}

NX_TEST(NX_MutexLockAndUnlock)
{
    NX_Mutex lock;
    
    NX_EXPECT_EQ(NX_MutexInit(&lock), NX_EOK);

    int i;
    for (i = 0; i < 12; i++)
    {
        NX_EXPECT_EQ(NX_MutexLock(&lock, NX_True), NX_EOK);
        NX_EXPECT_EQ(NX_MutexUnlock(&lock), NX_EOK);
    }
}

NX_TEST_TABLE(NX_Mutex)
{
    NX_TEST_UNIT(NX_MutexInit),
    NX_TEST_UNIT(NX_MutexLock),
    NX_TEST_UNIT(NX_MutexUnlock),
    NX_TEST_UNIT(NX_MutexLockAndUnlock),
};

NX_TEST_CASE(NX_Mutex);

#endif
