/**
 * Copyright (c) 2018-2021, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: page for each platform
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-9-18      JasonHu           Init
 */

#ifndef __PLATFORM_PAGE_ZONE__
#define __PLATFORM_PAGE_ZONE__

#include <xbook.h>

/**
 * Must define & init PageZone in each paltfrom 
 */
enum NX_PageZone
{
    NX_PAGE_ZONE_NORMAL = 0,
    NX_PAGE_ZONE_DMA,
    NX_PAGE_ZONE_USER,
    NX_PAGE_ZONE_NR,  /* Must be last member */
};
typedef enum NX_PageZone NX_PageZone;

NX_PUBLIC void HAL_PageZoneInit(void);

#endif  /* __PLATFORM_PAGE_ZONE__ */
