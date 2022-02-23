/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Buddy system
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-17     JasonHu           Update code style
 */

#include "buddy_common.h"
#include <mm/buddy.h>

#define NX_PAGE_INVALID_ORDER (-1)

/* page flags */
#define NX_PAGE_FLAG_IS_FREE  0x01

NX_PRIVATE void BuddyAddPage(NX_BuddySystem* system, NX_Page* page)
{
    NX_ASSERT(system && page);

    int order = page->order;
    page->flags |= NX_PAGE_FLAG_IS_FREE;

    NX_ListAddTail(&page->list, &system->pageBuddy[order]);
    system->bitmap |= 1UL << order;
    system->count[order]++;
}

NX_PRIVATE void BuddyDelPage(NX_BuddySystem* system, NX_Page* page)
{
    NX_ASSERT(system && page);

    int order = page->order;
    NX_ListDel(&page->list);
    page->flags &= ~NX_PAGE_FLAG_IS_FREE;

    if (NX_ListEmpty(&system->pageBuddy[order]))
        system->bitmap &= ~(1UL << order);
    system->count[order]--;
}

NX_PRIVATE void *PageToPtr(NX_BuddySystem* system, NX_Page* page)
{
    NX_PtrDiff diff = page - system->map;
    return &NX_ARRAY_CAST(system->pageStart, NX_PAGE_SIZE)[diff];
}

NX_Page* NX_PageFromPtr(NX_BuddySystem* system, void *ptr)
{
    NX_ASSERT(0 == ((NX_Size)ptr % NX_PAGE_SIZE));
    NX_PtrDiff diff = NX_ARRAY_CAST(ptr, NX_PAGE_SIZE) - NX_ARRAY_CAST(system->pageStart, NX_PAGE_SIZE);
    return &system->map[diff];
}

NX_PRIVATE NX_Size PageToPFN(NX_BuddySystem* system, NX_Page* page)
{
    NX_ASSERT(system);

    NX_PtrDiff diff = page - system->map;
    return diff;
}

NX_USED NX_PRIVATE NX_Page* PageFromPFN(NX_BuddySystem* system, NX_Size pfn)
{
    NX_ASSERT(system);
    return &system->map[pfn];
}

NX_PRIVATE int IsValidPFN(NX_BuddySystem* system, NX_Size pfn)
{
    NX_ASSERT(system);

    return pfn <= system->maxPFN;
}

NX_PRIVATE int DoPageFree(NX_Page* page)
{
    return (page->flags & NX_PAGE_FLAG_IS_FREE);
}

NX_PRIVATE NX_BuddySystem* BuddyCreateFromMemory(void *mem)
{
    NX_ASSERT(mem);
    NX_BuddySystem* system = mem;
    int order;
    for (order = 0; order <= NX_MAX_PAGE_ORDER; order++)
    {
        NX_ListInit(&system->pageBuddy[order]);
        system->count[order] = 0UL;
    }
    system->bitmap = 0UL;
    system->pageStart = NX_NULL;

    return system;
}

NX_BuddySystem* NX_BuddyCreate(void *mem, NX_Size size)
{
    NX_ASSERT(mem && size);
    NX_LOG_I("mem: 0x%p size: 0x%p", mem, size);
    if (!(mem && size))
    {
        return NX_NULL;
    }

    // Alloc NX_BuddySystem
    NX_BuddySystem* system = BuddyCreateFromMemory(mem);
    mem += sizeof(NX_BuddySystem);
    size -= sizeof(NX_BuddySystem);

    NX_Size page_count = size >> NX_PAGE_SHIFT;

    // Alloc page map
    NX_Size map_size = BuddyAlignUp(page_count * sizeof(NX_Page), NX_PAGE_SIZE);
    page_count -= map_size / NX_PAGE_SIZE;
    mem += map_size;
    size -= map_size;

    // Alloc page
    NX_Size mem_diff = (NX_TYPE_CAST(NX_PtrDiff, BuddyAlignPtr(mem, NX_PAGE_SIZE)) - NX_TYPE_CAST(NX_PtrDiff, mem));
    mem += mem_diff;
    size -= mem_diff;
    page_count = size >> NX_PAGE_SHIFT;

    system->pageStart = mem;
    system->maxPFN = page_count - 1;

    BUDDY_ASSERT(((NX_Size)mem & NX_PAGE_MASK) == 0, "must align to PAGE_SIZE");

    NX_Size i;
    for (i = 0; i < page_count; i++)
    {
        system->map[i].order = NX_PAGE_INVALID_ORDER;
    }

    int order = NX_PAGE_INVALID_ORDER;

    NX_Size count;
    for (i = 0, count = page_count; count; i += 1UL << order, count -= 1UL << order)
    {
        order = BuddyFlsSizet(count);
        NX_ASSERT(order >= 0);

        if (order > NX_MAX_PAGE_ORDER)
            order = NX_MAX_PAGE_ORDER;

        NX_Page* p = &system->map[i];
        NX_ListInit(&p->list);
        p->order = order;
        BuddyAddPage(system, p);
    }

    return system;
}

NX_PRIVATE NX_Page* BuddyLocateFree(NX_BuddySystem* system, int order)
{
    NX_ASSERT(system && (order >= 0));
    NX_ASSERT(NX_ListEmpty(&system->pageBuddy[order]) == (system->count[order] == 0));

    if (NX_ListEmpty(&system->pageBuddy[order]))
    {
        NX_Size bitmap = system->bitmap & (~0UL << order);
        if (!bitmap)
        {
            NX_LOG_E("Cannot find free page!");
            return NX_NULL;
        }

        order = BuddyFfsSizet(bitmap);
        NX_ASSERT(order >= 0);
    }

    return NX_ListLastEntry(&system->pageBuddy[order], NX_Page, list);
}

NX_PRIVATE void PageSplit(NX_BuddySystem* system, NX_Page* page, int order)
{
    NX_ASSERT(system && page && (order >= 0));

    int i;
    for (i = page->order; i > order; i--)
    {
        int new = i - 1;
        NX_Page* buddy = &page[1UL << new];
        buddy->order = new;
        BuddyAddPage(system, buddy);
    }
    page->order = order;
}

NX_PRIVATE NX_Page* PageMerge(NX_BuddySystem* system, NX_Page* page)
{
    NX_ASSERT(system && page);

    int order = page->order;
    NX_Size pfn = PageToPFN(system, page);

    for (; order <= NX_MAX_PAGE_ORDER;)
    {
        NX_Size buddyPFN = pfn ^ (1UL << order);
        NX_Page* buddy = page + (buddyPFN - pfn);

        if (!IsValidPFN(system, buddyPFN))
            break;
        if (buddy->order != order)
            break;
        if (!DoPageFree(buddy))
            break;

        BuddyDelPage(system, buddy);

        NX_Size privatePFN = buddyPFN | pfn;
        NX_Page* private = page + (privatePFN - pfn);
        private->order = NX_PAGE_INVALID_ORDER;

        NX_Size combinedPFN = buddyPFN & pfn;
        page = page + (combinedPFN - pfn);
        pfn = combinedPFN;
        order++;
    }

    page->order = order;
    return page;
}

NX_PRIVATE void *PagePrepareUsed(NX_BuddySystem* system, NX_Page* page, int order)
{
    NX_ASSERT(system && page && (order >= 0));

    BuddyDelPage(system, page);

    if (page->order > order)
    {
        PageSplit(system, page, order);
    }
    NX_AtomicSet(&page->reference, 1);
    return PageToPtr(system, page);
}

void *NX_BuddyAllocPage(NX_BuddySystem* system, NX_Size count)
{
    NX_ASSERT(system && count);

    if (count == 0UL)
    {
        NX_LOG_E("count == 0");
        return NX_NULL;
    }

    int order = BuddyFlsSizet(count + ((1UL << BuddyFlsSizet(count)) - 1));

    NX_Page* page = BuddyLocateFree(system, order);

    if (!page)
    {
        NX_LOG_E("Failed to call BuddyLocateFree");
        return NX_NULL;
    }
    return PagePrepareUsed(system, page, order);
}

NX_Error NX_BuddyIncreasePage(NX_BuddySystem* system, void *ptr)
{
    NX_ASSERT(system && ptr);

    if (ptr)
    {
        NX_Page* page = NX_PageFromPtr(system, ptr);
        NX_AtomicInc(&page->reference);
        return NX_EOK;
    }
    return NX_EINVAL;
}

/**
 * free page, if reference > 1, dec reference and return NX_EAGAIN.
 * if reference = 0, free the page and return NX_EOK.
 * 
 * return others if failed!
 */
NX_Error NX_BuddyFreePage(NX_BuddySystem* system, void *ptr)
{
    NX_ASSERT(system && ptr);

    if (ptr)
    {
        if (NX_PAGE_INVALID_ADDR(system, ptr))
        {
            return NX_EINVAL;
        }

        NX_Page* page = NX_PageFromPtr(system, ptr);

        if (DoPageFree(page))
        {
            NX_LOG_E("Double free!");
            return NX_EFAULT;
        }
        NX_ASSERT(NX_AtomicGet(&page->reference) > 0);

        NX_AtomicDec(&page->reference);

        if (NX_AtomicGet(&page->reference) > 0)    
        {
            return NX_EAGAIN;   /* need free again, but free success */
        }

        /* do real page free when ref zero */
        page = PageMerge(system, page);
        BuddyAddPage(system, page);        
        return NX_EOK;
    }
    return NX_EINVAL;
}
