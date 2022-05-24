/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: spin lock test 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-13     JasonHu           Init
 */

#include <base/spin.h>
#include <test/utest.h>

#ifdef CONFIG_NX_UTEST_SCHED_SPIN

NX_TEST(NX_SpinInit)
{
    NX_Spin lock;
    NX_EXPECT_NE(NX_SpinInit(NX_NULL), NX_EOK);
    NX_EXPECT_EQ(NX_SpinInit(&lock), NX_EOK);
    NX_EXPECT_NE(NX_SpinInit(&lock), NX_EOK);
}

NX_TEST(NX_SpinLock)
{
    NX_Spin lock;
    NX_Spin lockNoInit;

    NX_EXPECT_EQ(NX_SpinInit(&lock), NX_EOK);

    NX_EXPECT_NE(NX_SpinLock(NX_NULL), NX_EOK);
    NX_EXPECT_NE(NX_SpinLock(&lockNoInit), NX_EOK);

    NX_EXPECT_EQ(NX_SpinLock(&lock), NX_EOK);
    NX_EXPECT_NE(NX_SpinTryLock(&lock), NX_EOK);
}

NX_TEST(NX_SpinUnlock)
{
    NX_Spin lock;
    NX_Spin lockNoInit;
    
    NX_EXPECT_EQ(NX_SpinInit(&lock), NX_EOK);

    NX_EXPECT_NE(NX_SpinUnlock(NX_NULL), NX_EOK);
    NX_EXPECT_NE(NX_SpinUnlock(&lockNoInit), NX_EOK);

    NX_EXPECT_EQ(NX_SpinLock(&lock), NX_EOK);
    NX_EXPECT_EQ(NX_SpinUnlock(&lock), NX_EOK);
}

NX_TEST(NX_SpinLockAndUnlock)
{
    NX_Spin lock;
    
    NX_EXPECT_EQ(NX_SpinInit(&lock), NX_EOK);

    int i;
    for (i = 0; i < 12; i++)
    {
        NX_EXPECT_EQ(NX_SpinLock(&lock), NX_EOK);
        NX_EXPECT_EQ(NX_SpinUnlock(&lock), NX_EOK);
    }
}

NX_TEST_TABLE(NX_Spin)
{
    NX_TEST_UNIT(NX_SpinInit),
    NX_TEST_UNIT(NX_SpinLock),
    NX_TEST_UNIT(NX_SpinUnlock),
    NX_TEST_UNIT(NX_SpinLockAndUnlock),
};

NX_TEST_CASE(NX_Spin);

#endif
