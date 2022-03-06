/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: utest self test 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-2      JasonHu           Init
 */

#include <mods/test/utest.h>

#ifdef CONFIG_NX_UTEST_MODS_UTEST

NX_TEST(Test)
{
    NX_ASSERT_TRUE(1);
    NX_ASSERT_TRUE(NX_True);
    NX_ASSERT_FALSE(0);
    NX_ASSERT_FALSE(NX_False);
}

NX_TEST(Test1)
{
    NX_ASSERT_EQ(1, 1);
    NX_ASSERT_NE(0, 1);
    
    NX_ASSERT_LT(3, 5);
    NX_ASSERT_LE(3, 3);
    NX_ASSERT_LE(3, 4);
    NX_ASSERT_GT(4, 3);
    NX_ASSERT_GE(3, 3);
    
    char *p = NX_NULL;
    NX_ASSERT_NULL(p);
    char *q = (char *) 0x1000;
    NX_ASSERT_NOT_NULL(q);
}

NX_TEST(Test2)
{
    NX_ASSERT_IN_RANGE(1, 0, 5);
    NX_ASSERT_IN_RANGE(4, 0, 5);
    NX_ASSERT_IN_RANGE(1, 1, 5);
    NX_ASSERT_IN_RANGE(5, 1, 5);

    NX_ASSERT_NI_RANGE(1, 2, 5);
    NX_ASSERT_NI_RANGE(0, 1, 5);
    NX_ASSERT_NI_RANGE(6, 1, 5);
    NX_ASSERT_NI_RANGE(5, 1, 4);
}

NX_TEST(Test3)
{
    NX_ASSERT_STREQ("hello", "hello");
    char *s = "world!";
    NX_ASSERT_STREQ(s, "world!");

    char buf0[32] = {0x10, 0x20, 0x30,};
    char buf1[32] = {0x10, 0x20, 0x30,};
    NX_ASSERT_BUFEQ(buf0, buf1, 32);

    NX_ASSERT_STRNE("hello", "world");
    s = "hello!";
    NX_ASSERT_STRNE(s, "world!");

    char buf2[32] = {0x40, 0x50, 0x60,};
    NX_ASSERT_BUFNE(buf1, buf2, 32);
}

NX_TEST_SETUP(Test)
{
    NX_ASSERT_TRUE(1);
}

NX_TEST_CLEAN(Test)
{
    NX_ASSERT_FALSE(0);
}

NX_TEST_TABLE(UTest)
{
    NX_TEST_UNIT(Test),
    NX_TEST_UNIT_SETUP_CLEAN(Test),
    NX_TEST_UNIT(Test1),
    NX_TEST_UNIT(Test2),
    NX_TEST_UNIT(Test3),
};

NX_TEST_CASE_SETUP(UTest)
{
    NX_ASSERT_TRUE(1);
    return NX_EOK;
}

NX_TEST_CASE_CLEAN(UTest)
{
    NX_ASSERT_FALSE(0);
    return NX_EOK;
}

NX_TEST_CASE(UTest);

#endif
