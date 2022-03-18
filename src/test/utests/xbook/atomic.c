/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: atomic test 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-12     JasonHu           Init
 */

#include <xbook/atomic.h>
#include <test/utest.h>

#ifdef CONFIG_NX_UTEST_XBOOK_ATOMIC

NX_TEST(NX_AtomicSetAndGet)
{
    NX_Atomic val;
    NX_AtomicSet(&val, 0);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 0);
    
    NX_AtomicSet(&val, 1);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 1);
}

NX_TEST(NX_AtomicAdd)
{
    NX_Atomic val;
    NX_AtomicSet(&val, 0);
    NX_AtomicAdd(&val, 1);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 1);
    NX_AtomicAdd(&val, 3);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 4);
    NX_AtomicAdd(&val, 100);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 104);
}

NX_TEST(NX_AtomicSub)
{
    NX_Atomic val;
    NX_AtomicSet(&val, 0);
    NX_AtomicSub(&val, 1);
    NX_EXPECT_EQ(NX_AtomicGet(&val), -1);
    NX_AtomicSub(&val, 3);
    NX_EXPECT_EQ(NX_AtomicGet(&val), -4);
    NX_AtomicSub(&val, 100);
    NX_EXPECT_EQ(NX_AtomicGet(&val), -104);
}

NX_TEST(NX_AtomicInc)
{
    NX_Atomic val;
    NX_AtomicSet(&val, 0);
    NX_AtomicInc(&val);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 1);
    NX_AtomicInc(&val);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 2);
    NX_AtomicInc(&val);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 3);
}

NX_TEST(NX_AtomicDec)
{
    NX_Atomic val;
    NX_AtomicSet(&val, 0);
    NX_AtomicDec(&val);
    NX_EXPECT_EQ(NX_AtomicGet(&val), -1);
    NX_AtomicDec(&val);
    NX_EXPECT_EQ(NX_AtomicGet(&val), -2);
    NX_AtomicDec(&val);
    NX_EXPECT_EQ(NX_AtomicGet(&val), -3);
}

NX_TEST(NX_AtomicSetMask)
{
    NX_Atomic val;
    NX_AtomicSet(&val, 0);
    NX_AtomicSetMask(&val, 0x01);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 0x01);
    NX_AtomicSetMask(&val, 0x02);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 0x03);
    NX_AtomicSetMask(&val, 0x10);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 0x13);
}

NX_TEST(NX_AtomicClearMask)
{
    NX_Atomic val;
    NX_AtomicSet(&val, 0xffffff);
    NX_AtomicClearMask(&val, 0x01);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 0xfffffe);
    NX_AtomicClearMask(&val, 0x02);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 0xfffffc);
    NX_AtomicClearMask(&val, 0x10);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 0xffffec);
}

NX_TEST(NX_AtomicSwap)
{
    NX_Atomic val;
    NX_AtomicSet(&val, 0);
    NX_EXPECT_EQ(NX_AtomicSwap(&val, 1), 0);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 1);

    NX_EXPECT_EQ(NX_AtomicSwap(&val, 0x5a5a5a5a), 1);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 0x5a5a5a5a);
}

NX_TEST(NX_AtomicCAS)
{
    NX_Atomic val;
    NX_AtomicSet(&val, 0);
    NX_EXPECT_EQ(NX_AtomicCAS(&val, 0, 1), 0);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 1);

    NX_EXPECT_EQ(NX_AtomicCAS(&val, 1, 0x5a5a5a5a), 1);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 0x5a5a5a5a);

    NX_EXPECT_EQ(NX_AtomicCAS(&val, 1, 0xfafafa), 0x5a5a5a5a);
    NX_EXPECT_NE(NX_AtomicGet(&val), 0xfafafa);

    NX_EXPECT_EQ(NX_AtomicCAS(&val, 0x5a5a5a5a, 0xfafafa), 0x5a5a5a5a);
    NX_EXPECT_EQ(NX_AtomicGet(&val), 0xfafafa);
    
    NX_EXPECT_NE(NX_AtomicCAS(&val, 0, 1), 0);
    NX_EXPECT_NE(NX_AtomicGet(&val), 0);
}

NX_TEST_TABLE(NX_Atomic)
{
    NX_TEST_UNIT(NX_AtomicSetAndGet),
    NX_TEST_UNIT(NX_AtomicAdd),
    NX_TEST_UNIT(NX_AtomicSub),
    NX_TEST_UNIT(NX_AtomicInc),
    NX_TEST_UNIT(NX_AtomicDec),
    NX_TEST_UNIT(NX_AtomicSetMask),
    NX_TEST_UNIT(NX_AtomicClearMask),
    NX_TEST_UNIT(NX_AtomicSwap),
    NX_TEST_UNIT(NX_AtomicCAS),
};

NX_TEST_CASE(NX_Atomic);

#endif
