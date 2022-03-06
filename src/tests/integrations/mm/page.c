/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Page test
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-16     JasonHu           Init
 */

#include <mods/test/integration.h>
#include <mm/page.h>
#define NX_LOG_NAME "Page"
#include <utils/log.h>
#include <utils/memory.h>

#ifdef CONFIG_NX_TEST_INTEGRATION_PAGE

NX_INTEGRATION_TEST(NX_PageAlloc)
{
    /* alloc memory */
    void *bufs[16];

    int i;
    for (i = 0; i < 16; i++)
    {
        void *buf = NX_PageAlloc(i + 1);
        if (buf != NX_NULL)
        {
            NX_MemZero(buf, NX_PAGE_SIZE);
            NX_LOG_D("Alloc: %x", buf);
        }
        bufs[i] = buf; 
    }

    for (i = 0; i < 16; i++)
    {
        NX_LOG_D("Free: %x", bufs[i]);
        if (bufs[i])
        {
            NX_PageFree(bufs[i]);
        }
    }
    for (i = 0; i < 16; i++)
    {
        void *buf = NX_PageAlloc(i + 1);
        if (buf != NX_NULL)
        {
            NX_MemZero(buf, NX_PAGE_SIZE);
            NX_LOG_D("Alloc: %x", buf);
        }
        bufs[i] = buf; 
    }

#ifdef __I386__

    for (i = 0; i < 16; i++)
    {
        void *buf = NX_PageAllocInZone(NX_PAGE_ZONE_DMA, i + 1);
        if (buf != NX_NULL)
        {
            NX_MemZero(buf, NX_PAGE_SIZE);
            NX_LOG_D("Alloc: %x", buf);
        }
        bufs[i] = buf; 
    }

    for (i = 0; i < 16; i++)
    {
        NX_LOG_D("Free: %x", bufs[i]);
        if (bufs[i])
        {
            NX_PageFreeInZone(NX_PAGE_ZONE_DMA, bufs[i]);
        }
    }
    for (i = 0; i < 16; i++)
    {
        void *buf = NX_PageAllocInZone(NX_PAGE_ZONE_DMA, i + 1);
        if (buf != NX_NULL)
        {
            NX_MemZero(buf, NX_PAGE_SIZE);
            NX_LOG_D("Alloc: %x", buf);
        }
        bufs[i] = buf; 
    }
#endif

    return NX_EOK;
}

#endif
