/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: string test 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-2      JasonHu           Init
 * 2021-11-5      JasonHu           Add NX_StrCopy,NX_StrLen test
 */

#include <base/string.h>
#include <test/utest.h>

#ifdef CONFIG_NX_UTEST_UTILS_STRING

NX_TEST(NX_StrCmp)
{
    const char *str = "abc";
    
    NX_EXPECT_EQ(NX_StrCmp(str, "abc"), 0);
    NX_EXPECT_NE(NX_StrCmp(str, "cba"), 0);
    
    NX_EXPECT_EQ(NX_StrCmp(str, "abd"), -1);
    NX_EXPECT_EQ(NX_StrCmp(str, "abb"), 1);
}

NX_TEST(NX_StrCopy)
{
    const char *str = "hello!";
    char buf[10] = {0};
    NX_EXPECT_NOT_NULL(NX_StrCopy(buf, str));
    NX_EXPECT_STREQ(buf, str);
}

NX_TEST(NX_StrLen)
{
    const char *str = "hello!";
    NX_EXPECT_NE(NX_StrLen(str), 0);
    NX_EXPECT_EQ(NX_StrLen(str), 6);
}

NX_TEST_TABLE(String)
{
    NX_TEST_UNIT(NX_StrCmp),
    NX_TEST_UNIT(NX_StrCopy),
    NX_TEST_UNIT(NX_StrLen),
};

NX_TEST_CASE(String);

#endif
