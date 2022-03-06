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

#include <mods/test/integration.h>
#include <mm/heap_cache.h>
#define NX_LOG_NAME "TestHeapCache"
#include <utils/log.h>

#ifdef CONFIG_NX_TEST_INTEGRATION_HEAP_CACHE

NX_PRIVATE void LargeObjectTest(void)
{
    /* large object */
    void *p = NX_HeapAlloc(2 * NX_MB);
    if (p == NX_NULL)
    {
        NX_LOG_E("alloc failed!");
        return;
    }
    NX_HeapFree(p);
    p = NX_HeapAlloc(2 * NX_MB);
    if (p == NX_NULL)
    {
        NX_LOG_E("alloc failed!");
        return;
    }
    NX_HeapFree(p);   
}

NX_PRIVATE void MiddleObjectTest(void)
{
    /* middle object */
    void *p = NX_HeapAlloc(512 * NX_KB);
    if (p == NX_NULL)
    {
        NX_LOG_E("alloc failed!");
        return;
    }
    NX_HeapFree(p);
    
    p = NX_HeapAlloc(512 * NX_KB);
    if (p == NX_NULL)
    {
        NX_LOG_E("alloc failed!");
        return;
    }
    NX_HeapFree(p);

    void *buf[50];
    int i, j;
    for (i = 256 + 1, j = 0; i < 1024; i += 16, j++)
    {
        buf[j] = NX_HeapAlloc(i * NX_KB);
        NX_LOG_D("alloc: %p", buf[j]);
    }

    for (i = 256 + 1, j = 0; i < 1024; i += 16, j++)
    {
        if (buf[j] != NX_NULL)
        {
            NX_HeapFree(buf[j]);
            NX_LOG_D("free: %p", buf[j]);
        }
    }
}

NX_PRIVATE void SmallObjectTest(void)
{
    /* small object */
    void *p = NX_HeapAlloc(1);
    if (p != NX_NULL)
    {
        NX_LOG_D("Alloc & Free: %p", p);
        NX_HeapFree(p);
    }
    p = NX_HeapAlloc(8);
    if (p != NX_NULL)
    {
        NX_LOG_D("Alloc & Free: %p", p);
        NX_HeapFree(p);
    }
    p = NX_HeapAlloc(9);
    if (p != NX_NULL)
    {
        NX_LOG_D("Alloc & Free: %p", p);
        NX_HeapFree(p);
    }
    p = NX_HeapAlloc(16);
    if (p != NX_NULL)
    {
        NX_LOG_D("Alloc & Free: %p", p);
        NX_HeapFree(p);
    }
    p = NX_HeapAlloc(20);
    if (p != NX_NULL)
    {
        NX_LOG_D("Alloc & Free: %p", p);
        NX_HeapFree(p);
    }
    p = NX_HeapAlloc(28);
    if (p != NX_NULL)
    {
        NX_LOG_D("Alloc & Free: %p", p);
        NX_HeapFree(p);
    }
    p = NX_HeapAlloc(32);
    if (p != NX_NULL)
    {
        NX_LOG_D("Alloc & Free: %p", p);
        NX_HeapFree(p);
    }
    p = NX_HeapAlloc(48);
    if (p != NX_NULL)
    {
        NX_LOG_D("Alloc & Free: %p", p);
        NX_HeapFree(p);
    }
    p = NX_HeapAlloc(63);
    if (p != NX_NULL)
    {
        NX_LOG_D("Alloc & Free: %p", p);
        NX_HeapFree(p);
    }
    p = NX_HeapAlloc(72);
    if (p != NX_NULL)
    {
        NX_LOG_D("Alloc & Free: %p", p);
        NX_HeapFree(p);
    }
    p = NX_HeapAlloc(120);
    if (p != NX_NULL)
    {
        NX_LOG_D("Alloc & Free: %p", p);
        NX_HeapFree(p);
    }
    p = NX_HeapAlloc(250);
    if (p != NX_NULL)
    {
        NX_LOG_D("Alloc & Free: %p", p);
        NX_HeapFree(p);
    }
    int i, j;
    for (i = 0; i < 20; i++)
    {
        p = NX_HeapAlloc(512);
        if (p != NX_NULL)
        {
            NX_LOG_D("Alloc & Free 512: %p", p);
            NX_HeapFree(p);
        }
    }
    
    for (i = 0; i < 20; i++)
    {
        p = NX_HeapAlloc(2048);
        if (p != NX_NULL)
        {
            NX_LOG_D("Alloc & Free 2048: %p", p);
            NX_HeapFree(p);
        }
    }
    
    for (i = 1; i < 4096; i+=8)
    {
        for (j = 0; j < 10; j++)
        {
            p = NX_HeapAlloc(i);
            if (p != NX_NULL)
            {
                NX_LOG_D("Alloc & Free: %p NX_Size: %d", p, NX_HeapGetObjectSize(p));
                NX_HeapFree(p);
            }
        }
    }
}

NX_INTEGRATION_TEST(NX_HeapCache)
{
    LargeObjectTest();
    MiddleObjectTest();
    SmallObjectTest();
    return NX_EOK;
}

#endif
