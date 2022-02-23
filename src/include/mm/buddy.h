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

#ifndef __MM_BUDDY__
#define __MM_BUDDY__

#include <xbook.h>
#include <utils/list.h>
#include <mm/page.h>
#include <xbook/atomic.h>

#define NX_MAX_PAGE_ORDER 20

#define NX_PAGE_SHIFT_TO_MASK(s) ((1UL << s) - 1)
#define NX_PAGE_ORDER_MASK (NX_PAGE_SHIFT_TO_MASK((NX_MAX_PAGE_ORDER + NX_PAGE_SHIFT)) - NX_PAGE_MASK)

#define NX_PAGE_INVALID_ADDR(system, ptr) \
        ((ptr) < (system)->pageStart || \
        (ptr) > (void *)((NX_U8 *)(system)->pageStart + (system->maxPFN << NX_PAGE_SHIFT)))

struct NX_Page
{
    NX_List list;
    NX_U32 flags;
    NX_I32 order;
    NX_Size sizeClass;         /* size class on this span */
    NX_Atomic reference;        /* page reference */
};
typedef struct NX_Page NX_Page;

struct NX_BuddySystem
{
    NX_List pageBuddy[NX_MAX_PAGE_ORDER + 1];
    NX_Size count[NX_MAX_PAGE_ORDER + 1];
    NX_Size bitmap;    /* map order has free page */
    void *pageStart;    /* page start addr */
    NX_Size maxPFN;
    NX_Page map[0];    /* pages array */
};
typedef struct NX_BuddySystem NX_BuddySystem;

NX_PUBLIC NX_BuddySystem* NX_BuddyCreate(void *mem, NX_Size size);
NX_PUBLIC void *NX_BuddyAllocPage(NX_BuddySystem* system, NX_Size count);
NX_PUBLIC NX_Error NX_BuddyFreePage(NX_BuddySystem* system, void *ptr);
NX_PUBLIC NX_Error NX_BuddyIncreasePage(NX_BuddySystem* system, void *ptr);

NX_PUBLIC NX_Page* NX_PageFromPtr(NX_BuddySystem* system, void *ptr);

#endif /* __MM_BUDDY__ */
