/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Page heap test
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-16     JasonHu           Init
 */

#include <test/integration.h>
#include <mm/page_cache.h>
#include <mm/page.h>
#define NX_LOG_NAME "PageCache"
#include <utils/log.h>
#include <xbook/debug.h>
#include <utils/memory.h>

#ifdef CONFIG_NX_TEST_INTEGRATION_PAGE_CACHE

NX_PRIVATE void NX_PageCacheLarge(void)
{
    void *span = NX_PageCacheAlloc(128);
    if (span)
    {
        NX_LOG_D("span: %p", span);
        NX_PageCacheFree(span);
    }

    span = NX_PageCacheAlloc(128);
    if (span)
    {    
        NX_LOG_D("span: %p", span);
        NX_PageCacheFree(span);
    }

    span = NX_PageCacheAlloc(256);
    if (span)
    {
        NX_LOG_D("span: %p", span);
        NX_PageCacheFree(span + NX_PAGE_SIZE);
    }

    span = NX_PageCacheAlloc(256);
    if (span)
    {
        NX_LOG_D("span: %p", span);
        NX_PageCacheFree(span + NX_PAGE_SIZE);
    }

    void *table[100];
    
    int i;
    for (i = 0; i < 100; i++)
    {
        table[i] = NX_PageCacheAlloc(128 + i);
        if (table[i])
        {
            NX_LOG_D("alloc span: %p", table[i]);
            NX_MemSet(table[i], 0x5a, NX_PAGE_SIZE * 128 + i);
        }
    }

    for (i = 0; i < 100; i++)
    {
        if (table[i])
        {
            NX_LOG_D("free span: %p", table[i]);
            NX_PageCacheFree(table[i]);
        }
    }
    
}

NX_PRIVATE void NX_PageCacheSmall(void)
{
    void *span = NX_PageCacheAlloc(1);
    if (span)
    {
        NX_LOG_D("span: %p", span);
        NX_PageCacheFree(span);
    }
    span = NX_PageCacheAlloc(1);
    if (span)
    {
        NX_LOG_D("span: %p", span);
        NX_PageCacheFree(span);
    }
    span = NX_PageCacheAlloc(10);
    if (span)
    {
        NX_LOG_D("span: %p", span);
        NX_PageCacheFree(span);
    }
    span = NX_PageCacheAlloc(10);
    if (span)
    {
        NX_LOG_D("span: %p", span);
        NX_PageCacheFree(span);
    }

    void *table[128];
    
    int i;
    for (i = 0; i < 128; i++)
    {
        table[i] = NX_PageCacheAlloc(i + 1);
        if (table[i])
        {
            NX_ASSERT(table[i] != NX_NULL);
            NX_LOG_D("alloc span:%d-> %p", i, table[i]);
            NX_MemSet(table[i], 0x5a, NX_PAGE_SIZE * (i + 1));
        }
    }

    for (i = 0; i < 128; i++)
    {
        if (table[i])
        {
            NX_LOG_D("free span:%d-> %p", i, table[i]);
            NX_PageCacheFree(table[i]);
        }
    }

    for (i = 0; i < 128; i++)
    {
        table[i] = NX_PageCacheAlloc(i + 1);
        if (table[i])
        {
            NX_ASSERT(table[i] != NX_NULL);
            NX_LOG_D("alloc span:%d-> %p", i, table[i]);
            NX_MemSet(table[i], 0x5a, NX_PAGE_SIZE * (i + 1));
        }
    }

    for (i = 0; i < 128; i++)
    {
        if (table[i])
        {
            NX_LOG_D("free span:%d-> %p", i, table[i]);
            NX_PageCacheFree(table[i]);
        }
    }
}

NX_PRIVATE void NX_PageCacheOnePage(void)
{
    void *table[1024];
    int i;
    for (i = 0; i < 1024; i++)
    {
        table[i] = NX_PageCacheAlloc(1);
        if (table[i])
        {
            NX_LOG_D("alloc span: %p", table[i]);
            NX_MemSet(table[i], 0x5a, NX_PAGE_SIZE);
        }
    }

    for (i = 0; i < 1024; i++)
    {
        if (table[i])
        {
            NX_LOG_D("free span: %p", table[i]);
            NX_PageCacheFree(table[i]);
        }
    }
    for (i = 0; i < 1024; i++)
    {
        table[i] = NX_PageCacheAlloc(1);
        if (table[i])
        {
            NX_LOG_D("alloc span: %p", table[i]);
            NX_MemSet(table[i], 0x5a, NX_PAGE_SIZE);
        }
    }

    for (i = 0; i < 1024; i++)
    {
        if (table[i])
        {
            NX_LOG_D("free span: %p", table[i]);
            NX_PageCacheFree(table[i]);
        }
    }
}

NX_INTEGRATION_TEST(NX_PageCacheTest)
{
    NX_LOG_D("===>PageCacheLarge");
    NX_PageCacheLarge();
    NX_LOG_D("===>PageCacheSmall");
    NX_PageCacheSmall();
    NX_LOG_D("===>PageCacheOnePage");
    NX_PageCacheOnePage();
    return NX_EOK;
}

#endif
