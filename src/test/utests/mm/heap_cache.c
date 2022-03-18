/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: heap cache test 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-5      JasonHu           Init
 */

#include <test/utest.h>
#include <mm/heap_cache.h>
#include <utils/memory.h>

#ifdef CONFIG_NX_UTEST_HEAP_CACHE

NX_TEST(HeapAllocAndFree)
{
    NX_ASSERT_NULL(NX_HeapAlloc(0));
    NX_ASSERT_EQ(NX_HeapFree(NX_NULL), NX_EINVAL);

    void *p = NX_HeapAlloc(32);
    NX_ASSERT_NOT_NULL(p);
    NX_MemZero(p, 32);
    NX_ASSERT_EQ(NX_HeapFree(p), NX_EOK);

    p = NX_HeapAlloc(256);
    NX_ASSERT_NOT_NULL(p);
    NX_MemZero(p, 256);
    NX_ASSERT_EQ(NX_HeapFree(p), NX_EOK);

    p = NX_HeapAlloc(128 * 1024);
    NX_ASSERT_NOT_NULL(p);
    NX_MemZero(p, 128 * 1024);
    NX_ASSERT_EQ(NX_HeapFree(p), NX_EOK);

    p = NX_HeapAlloc(512 * 1024);
    NX_ASSERT_NOT_NULL(p);
    NX_MemZero(p, 512 * 1024);
    NX_ASSERT_EQ(NX_HeapFree(p), NX_EOK);

    p = NX_HeapAlloc(1024*1024*4);
    NX_EXPECT_NOT_NULL(p);
    NX_EXPECT_EQ(NX_HeapFreeSatety(p), NX_EOK);
    NX_EXPECT_NE(NX_HeapFree(p), NX_EOK);
}

NX_TEST_TABLE(NX_HeapCache)
{
    NX_TEST_UNIT(HeapAllocAndFree),
};

NX_TEST_CASE(NX_HeapCache);

#endif
