/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Buddy system
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-17     JasonHu           Init
 */

#ifndef __MM_PAGE__
#define __MM_PAGE__

#include <xbook.h>
#include <page_zone.h>   /* Platfrom page */

#ifdef CONFIG_NX_PAGE_SHIFT
#define NX_PAGE_SHIFT CONFIG_NX_PAGE_SHIFT
#else
#define NX_PAGE_SHIFT 12
#endif

#define NX_PAGE_SIZE   (1U << NX_PAGE_SHIFT)
#define NX_PAGE_MASK   (NX_PAGE_SIZE - 1UL)

#define NX_PAGE_UMASK  (~NX_PAGE_MASK)
#define NX_PAGE_ADDR_MASK NX_PAGE_UMASK

#define NX_PAGE_ALIGNUP(value) (((value) + NX_PAGE_MASK) & NX_PAGE_UMASK)
#define NX_PAGE_ALIGNDOWN(value) ((value) & NX_PAGE_UMASK)

NX_PUBLIC void NX_PageInitZone(NX_PageZone zone, void *mem, NX_Size size);
NX_PUBLIC void *NX_PageAllocInZone(NX_PageZone zone, NX_Size count);
NX_PUBLIC NX_Error NX_PageFreeInZone(NX_PageZone zone, void *ptr);
NX_PUBLIC NX_Error NX_PageIncreaseInZone(NX_PageZone zone, void *ptr);
NX_PUBLIC void *NX_PageZoneGetBase(NX_PageZone zone);
NX_PUBLIC NX_Size NX_PageZoneGetPages(NX_PageZone zone);

NX_PUBLIC void *NX_PageZoneGetBuddySystem(NX_PageZone zone);

#define NX_PageAlloc(count) NX_PageAllocInZone(NX_PAGE_ZONE_NORMAL, count)
#define NX_PageFree(ptr) NX_PageFreeInZone(NX_PAGE_ZONE_NORMAL, ptr)
#define NX_PageIncrease(ptr) NX_PageIncreaseInZone(NX_PAGE_ZONE_NORMAL, ptr)

#define NX_Phy2Virt(addr) ((addr) + NX_KVADDR_OFFSET)
#define NX_Virt2Phy(addr) ((addr) - NX_KVADDR_OFFSET)

#endif /* __MM_PAGE__ */
