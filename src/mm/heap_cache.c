/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Heap Cache
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-25     JasonHu           Update code style
 */

#include <mm/heap_cache.h>
#include <utils/math.h>
#include <utils/memory.h>
#include <mm/buddy.h>
#include <mm/page.h>
#include <sched/mutex.h>

#define NX_LOG_LEVEL NX_LOG_INFO
#define NX_LOG_NAME "HeapCache"
#include <utils/log.h>
#include <xbook/debug.h>

/**
 * size class -> span pages
 * ----------------------------
 * 16~1024 -> 1 pages
 * 1024~8192 -> 4 pages
 * 8192~36864 -> 18 pages
 * 36864~104096 -> 52 pages
 * 104096~262144 -> 128 pages
 * N pages -> N pages
 */

/**
 * size class:
 * ----------------------------
 * step 16B: 16, 32, 48, 64, 80, 96, 112, 128
 * step M -> AlignDown((N / 8), 2^n)
 */

#define MAX_SIZE_CLASS_VALUE 16

#define MAX_SIZE_CLASS_NR 96

#define MAX_SMALL_OBJECT_SIZE (256 * NX_KB)
#define MAX_SMALL_SPAN_THRESOLD 8

#define MAX_MIDDLE_OBJECT_SIZE (1 * NX_MB)
#define MAX_MIDDLE_OBJECT_THRESOLD 32

NX_PRIVATE struct NX_HeapSizeClass CacheSizeAarray[MAX_SIZE_CLASS_NR];
NX_PRIVATE NX_HeapCache MiddleSizeCache;

NX_PRIVATE NX_USize AlignDownToPow2(NX_USize size)
{
    NX_USize n = 19;    /* pow(2, 19) -> 512kb */
    while (NX_PowInt(2, n) > size)
    {
        n--;
    }
    return NX_PowInt(2, n);
}

NX_PRIVATE NX_USize SizeToPageCount(NX_USize size)
{
    if (size > 0 && size <= 1024)
    {
        return 1;
    }
    else if (size > 1024 && size <= 8192)
    {
        return 4;
    }
    else if (size > 8192 && size <= 36864)
    {
        return 18;
    }
    else if (size > 36864 && size <= 104096)
    {
        return 52;
    }
    else if (size > 104096 && size <= 262144)
    {
        return 256;
    }
    else
    {
        return NX_DIV_ROUND_UP(size, NX_PAGE_SIZE);
    }
}

NX_PRIVATE void HeapCacheInitOne(NX_HeapCache *cache, NX_USize classSize)
{
    cache->classSize = classSize;
    NX_ListInit(&cache->objectFreeList);
    cache->objectFreeCount = 0;
    NX_MutexInit(&cache->lock);
}

NX_PRIVATE void HeapSizeClassInit(void)
{
    int n = 0;

    int i;

    CacheSizeAarray[n].size = MAX_SIZE_CLASS_VALUE;
    n++;

    for (i = 32; i <= 128; i+=16)
    {
        CacheSizeAarray[n].size = i;
        n++;
    }
    for (i = 128 + 128 / 8; i <= MAX_SMALL_OBJECT_SIZE; )
    {
        CacheSizeAarray[n].size = i;
        i += AlignDownToPow2(i / 8);
        n++;
    }

    for (i = 0; i < MAX_SIZE_CLASS_NR; i++)
    {
        HeapCacheInitOne(&CacheSizeAarray[i].cache, CacheSizeAarray[i].size);
    }

    HeapCacheInitOne(&MiddleSizeCache, 0);
}

NX_INLINE NX_HeapCache *SizeToCache(NX_USize size)
{
    NX_ASSERT(size <= MAX_SMALL_OBJECT_SIZE);
    NX_HeapCache *cache = NX_NULL;
    int index;

    for (index = 0; index < MAX_SIZE_CLASS_NR; index++)
    {
        if (CacheSizeAarray[index].size >= size)
        {
            /* alloc in this cache */
            cache = &CacheSizeAarray[index].cache;
            break;
        }
    }
    return cache;
}

NX_PRIVATE NX_HeapSmallCacheSystem *GetFreeSmallCacheSystem(NX_HeapCache *cache)
{
    NX_HeapSmallCacheSystem *system;
    NX_PageSpan *span;
    NX_ListForEachEntry(span, &cache->objectFreeList, list)
    {
        system = (NX_HeapSmallCacheSystem *)span;
        if (!NX_ListEmpty(&system->objectFreeList))
        {
            return system;
        }
    }
    return NX_NULL;
}

NX_PRIVATE void *GetFreeSmallCacheObject(NX_HeapSmallCacheSystem *system, NX_USize size)
{
    NX_HeapSmallCacheObject *object;
    object = NX_ListFirstEntryOrNULL(&system->objectFreeList, NX_HeapSmallCacheObject, list);
    if (object == NX_NULL)
    {
        return NX_NULL;
    }
    --system->objectFreeCount;
    NX_ListDelInit(&object->list); /* del from free list */
    NX_MemZero(object, size);
    return object;
}

NX_PRIVATE void PageNodeMarkSize(NX_PageSpan *spanNode, NX_USize size)
{
    /* NOTICE: must get page node from span page */
    NX_Page *pageNode = NX_PageFromPtr(NX_PageZoneGetBuddySystem(NX_PAGE_ZONE_NORMAL), spanNode);
    NX_ASSERT(pageNode != NX_NULL);
    /* mark size class on the span */
    pageNode->sizeClass = size;
}

NX_PRIVATE NX_Error PutFreeSmallCacheObject(NX_HeapCache *cache, NX_HeapSmallCacheSystem *system, void *ptr)
{
    NX_HeapSmallCacheObject *object;

    /* if length is too long and cache free object over thresold, free this system */
    if (system->objectFreeCount + 1 >= system->maxObjects &&
        cache->objectFreeCount + 1 >= MAX_SMALL_SPAN_THRESOLD)
    {
        NX_PageSpan *spanNode = &system->pageSpan;
        
        /* del span from free list */
        NX_ListDel(&spanNode->list);
        --cache->objectFreeCount;

        /* clear page node size with span node */
        PageNodeMarkSize(spanNode, 0);

        /* free span */
        return NX_PageCacheFree(spanNode);
    }
    else
    {
        /* free to free list */
        object = (NX_HeapSmallCacheObject *)ptr;
        ++system->objectFreeCount;
        NX_ListAddTail(&object->list, &system->objectFreeList);
        return NX_EOK;
    }
}

NX_PRIVATE void BuildSmallCacheSystem(NX_PageSpan *spanNode, NX_USize objectCount, NX_USize size)
{
    NX_HeapSmallCacheObject *objectNode;
    NX_HeapSmallCacheSystem *system = (NX_HeapSmallCacheSystem *)spanNode;
    NX_ListInit(&system->objectFreeList);
    system->objectFreeCount = 0;

    /* skip system self */
    NX_USize systemUsedObjectCount = NX_DIV_ROUND_UP(sizeof(NX_HeapSmallCacheSystem), size);

    /* update object count */
    objectCount -= systemUsedObjectCount;
    system->maxObjects = objectCount;

    /* split span to object */
    NX_U8 *start = (NX_U8 *)spanNode + (systemUsedObjectCount * size);
    int i;
    for (i = 0; i < objectCount; i++)
    {
        objectNode = (NX_HeapSmallCacheObject *)start;
        NX_ListAddTail(&objectNode->list, &system->objectFreeList);
        ++system->objectFreeCount;
        start += size;
    }
    NX_ASSERT(system->objectFreeCount >= 1);
    NX_ASSERT(system->objectFreeCount == objectCount);
}

NX_PRIVATE NX_Error AllocSpan(NX_HeapCache *cache, NX_USize pageCount, NX_USize objectCount, NX_USize size)
{
    NX_PageSpan *spanNode = (NX_PageSpan *)NX_PageCacheAlloc(pageCount);
    if (spanNode == NX_NULL)
    {
        return NX_ENOMEM;
    }
    NX_MemZero(spanNode, pageCount * NX_PAGE_SIZE);

    /* add span to free list */
    spanNode->pageCount = pageCount;
    ++cache->objectFreeCount;
    NX_ListAdd(&spanNode->list, &cache->objectFreeList);

    /* build small cache system */
    if (cache != &MiddleSizeCache)
    {
        BuildSmallCacheSystem(spanNode, objectCount, size);
    }

    /* mark page node size with span node */
    PageNodeMarkSize(spanNode, size);
    return NX_EOK;
}

NX_PRIVATE void *AllocSmallObject(NX_HeapCache *cache, NX_USize pageCount, NX_USize objectCount, NX_USize size)
{
    NX_HeapSmallCacheSystem *system = NX_NULL;

    do
    {
        system = GetFreeSmallCacheSystem(cache);
        if (system == NX_NULL)
        {
            /* need build a new system */
            if (AllocSpan(cache, pageCount, objectCount, size) != NX_EOK)
            {
                return NX_NULL;
            }
            NX_ASSERT(cache->objectFreeCount > 0);
            NX_ASSERT(!NX_ListEmpty(&cache->objectFreeList));
        }
    } while (system == NX_NULL);

    NX_ASSERT(system);
    /* get a object from system */
    return GetFreeSmallCacheObject(system, size);
}

NX_PRIVATE NX_Error FreeSmallObject(void *span, void *object)
{
    NX_HeapSmallCacheSystem *system;
    NX_Error err;
    NX_USize sizeClass;
    NX_HeapCache *cache;
    NX_Page *pageNode;

    /* NOTICE: must get page node from span page */
    pageNode = NX_PageFromPtr(NX_PageZoneGetBuddySystem(NX_PAGE_ZONE_NORMAL), span);
    NX_ASSERT(pageNode != NX_NULL);

    /* get class size from page */
    sizeClass = pageNode->sizeClass;

    cache = SizeToCache(sizeClass);
    NX_ASSERT(cache != NX_NULL);

    /* locate small object system */
    system = (NX_HeapSmallCacheSystem *)span;

    NX_MutexLock(&cache->lock, NX_True);
    err = PutFreeSmallCacheObject(cache, system, object);
    NX_MutexUnlock(&cache->lock);
    return err;
}

NX_PUBLIC void *NX_HeapAlloc(NX_USize size)
{
    NX_USize pageCount;
    NX_HeapCache *cache = NX_NULL;
    NX_PageSpan *spanNode = NX_NULL;
    void *memObject = NX_NULL;
    NX_USize objectCount;

    if (!size)
    {
        return NX_NULL;
    }

    /* size align up with 16 */
    size = NX_ALIGN_UP(size, MAX_SIZE_CLASS_VALUE);

    pageCount = SizeToPageCount(size);

    /* get cache by size */
    if (size > MAX_SMALL_OBJECT_SIZE)   /* alloc big span */
    {
        if (size > MAX_MIDDLE_OBJECT_SIZE) /* alloc directly from page cache */
        {
            memObject = NX_PageCacheAlloc(NX_DIV_ROUND_UP(size, NX_PAGE_SIZE));
            if (memObject == NX_NULL)
            {
                return NX_NULL;
            }
            spanNode = (NX_PageSpan *)memObject;
            PageNodeMarkSize(spanNode, size);
            return memObject;
        }
        else    /* alloc from middle cache */
        {
            cache = &MiddleSizeCache;
        }
    }
    else
    {
        cache = SizeToCache(size);
    }
    NX_ASSERT(cache != NX_NULL);
    
    objectCount = NX_DIV_ROUND_DOWN(pageCount * NX_PAGE_SIZE, size); 

    NX_MutexLock(&cache->lock, NX_True);
    if (NX_ListEmpty(&cache->objectFreeList)) /* no object, need alloc from page cache */
    {
        if (AllocSpan(cache, pageCount, objectCount, size) != NX_EOK)
        {
            NX_MutexUnlock(&cache->lock);
            return NX_NULL;
        }
    }
    NX_ASSERT(cache->objectFreeCount > 0);
    NX_ASSERT(!NX_ListEmpty(&cache->objectFreeList));

    /* get a object from list */
    if (cache == &MiddleSizeCache)
    {
        spanNode = NX_ListFirstEntry(&cache->objectFreeList, NX_PageSpan, list);
        --cache->objectFreeCount;
        NX_ListDel(&spanNode->list); /* del from free list */
        memObject = (void *)spanNode;
    }
    else
    {
        /* alloc from small cache */
        memObject = AllocSmallObject(cache, pageCount, objectCount, size);
    }
    NX_MutexUnlock(&cache->lock);
    return (void *)memObject;
}

NX_PUBLIC NX_Error NX_HeapFree(void *object)
{
    /* object to page, then to span */
    void *page = NX_NULL;
    void *span = NX_NULL;
    NX_HeapCache *cache = NX_NULL;
    NX_USize sizeClass;

    if (object == NX_NULL) /* can't free NX_NULL object */
    {
        return NX_EINVAL;
    }

    page = (void *)(((NX_Addr) object) & NX_PAGE_UMASK);
    span = NX_PageToSpan(page);
    if (span == NX_NULL)
    {
        return NX_EINVAL;
    }
    
    /* get size class */
    sizeClass = NX_HeapGetObjectSize(object);
    if (!sizeClass)
    {
        return NX_EFAULT;
    }

    /* get cache by size */
    if (sizeClass > MAX_SMALL_OBJECT_SIZE)   /* free to big span */
    {
        if (sizeClass > MAX_MIDDLE_OBJECT_SIZE) /* free directly to page cache */
        {
            return NX_PageCacheFree(span);
        }
        else    /* free to middle cache */
        {
            cache = &MiddleSizeCache;

            NX_MutexLock(&cache->lock, NX_True);
            /* if length is too long, free to page cache */
            if (cache->objectFreeCount + 1 >= MAX_MIDDLE_OBJECT_THRESOLD) 
            {
                PageNodeMarkSize(span, 0);
                NX_MutexUnlock(&cache->lock);
                return NX_PageCacheFree(span);
            }
            else    /* free to free list */
            {
                NX_PageSpan *spanNode = (NX_PageSpan *) span;
                ++cache->objectFreeCount;
                NX_ListAdd(&spanNode->list, &cache->objectFreeList);
            }
            NX_MutexUnlock(&cache->lock);
            return NX_EOK;
        }
    }

    /* free to small cache */
    return FreeSmallObject(span, object);
}

NX_PUBLIC NX_USize NX_HeapGetObjectSize(void *object)
{
    if (object == NX_NULL) /* can't free NX_NULL object */
    {
        return 0;
    }
    /* object to page, then to span */
    void *page = (void *)(((NX_Addr) object) & NX_PAGE_UMASK);
    void *span = NX_PageToSpan(page);
    if (span == NX_NULL)
    {
        return 0;
    }
    
    /* NOTICE: page must be span page */
    NX_Page *pageNode = NX_PageFromPtr(NX_PageZoneGetBuddySystem(NX_PAGE_ZONE_NORMAL), span);
    NX_ASSERT(pageNode != NX_NULL);
    /* get class size from page */
    return pageNode->sizeClass;
}

NX_PUBLIC void NX_HeapCacheInit(void)
{
    HeapSizeClassInit();
}
