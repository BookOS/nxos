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
#include <mm/page_cache.h>
#include <mm/buddy.h>
#include <mm/page.h>
#include <sched/mutex.h>

#define NX_LOG_LEVEL NX_LOG_INFO
#define NX_LOG_NAME "HeapCache"
#include <utils/log.h>
#include <xbook/debug.h>

/**
 * size class => span pages
 * ----------------------------
 * 8~1024 => 1 pages
 * 1024~8192 => 4 pages
 * 8192~36864 => 18 pages
 * 36864~104096 => 52 pages
 * 104096~262144 => 128 pages
 * N pages => N pages
 */

/**
 * size class:
 * ----------------------------
 * step 8B: 8, 16  
 * step 16B: 32, 48, 64, 80, 96, 112, 128
 * step M -> AlignDown((N / 8), 2^n)
 */

#define MAX_SIZE_CLASS_NR 97

#define MAX_SMALL_OBJECT_SIZE (256 * NX_KB)
#define MAX_SMALL_SPAN_THRESOLD 8

#define MAX_MIDDLE_OBJECT_SIZE (1 * NX_MB)
#define MAX_MIDDLE_OBJECT_THRESOLD 32

NX_PRIVATE struct NX_SizeClass CacheSizeAarray[MAX_SIZE_CLASS_NR];
NX_PRIVATE NX_HeapCache MiddleSizeCache;
NX_PRIVATE NX_Mutex HeapCacheLock;

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
        return 128;
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
    NX_ListInit(&cache->spanFreeList);
    NX_AtomicSet(&cache->spanFreeCount, 0);
    NX_AtomicSet(&cache->objectFreeCount, 0);
}

NX_PRIVATE void HeapSizeClassInit(void)
{
    int n = 0;

    int i;
    for (i = 8; i <= 16; i+=8)
    {
        CacheSizeAarray[n].size = i;
        n++;
    }
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
    NX_U32 index = 0;

    for (; index < MAX_SIZE_CLASS_NR; index++)
    {
        if (CacheSizeAarray[index].size >= size)
        {
            cache = &CacheSizeAarray[index].cache;
            /* alloc in this cache */
            break;
        }
    }
    return cache;
}

NX_PRIVATE void *DoHeapAlloc(NX_USize size)
{
    /* size align up with 8 */
    size = NX_ALIGN_UP(size, 8);

    NX_PageSpan *span = NX_NULL;
    NX_USize pageCount = SizeToPageCount(size);
    NX_HeapCache *cache = NX_NULL;
    NX_SmallCacheObject *objectNode = NX_NULL;

    if (size > MAX_SMALL_OBJECT_SIZE)   /* alloc big span */
    {
        if (size > MAX_MIDDLE_OBJECT_SIZE) /* alloc directly from span */
        {
            return NX_PageCacheAlloc(NX_DIV_ROUND_UP(size, NX_PAGE_SIZE));
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
    NX_ASSERT(NX_AtomicGet(&cache->objectFreeCount) >= 0);
    if (NX_AtomicGet(&cache->objectFreeCount) == 0) /* no object, need split from span */
    {
        NX_USize objectCount;
        
        NX_ASSERT(NX_AtomicGet(&cache->spanFreeCount) >= 0);
        if (NX_AtomicGet(&cache->spanFreeCount) == 0)    /* no span, need alloc from page cache */
        {
            span = (NX_PageSpan *)NX_PageCacheAlloc(pageCount);
            if (span == NX_NULL)
            {
                NX_LOG_E("no enough memory span!");
                return NX_NULL;
            }
        }
        else    /* get span from span list */
        {
            span = NX_ListFirstEntry(&cache->spanFreeList, NX_PageSpan, list);
            NX_ListDelInit(&span->list);
            NX_AtomicDec(&cache->spanFreeCount);
        }

        if (cache == &MiddleSizeCache)  /* return span if middle cache */
        {
            return (void *)span;
        }

        /* split to object free list */
        /* NOTICE: must get page node from span page */
        NX_Page *pageNode = NX_PageFromPtr(NX_PageZoneGetBuddySystem(NX_PAGE_ZONE_NORMAL), span);
        NX_ASSERT(pageNode != NX_NULL);

        objectCount = NX_DIV_ROUND_DOWN(pageCount * NX_PAGE_SIZE, size); 

        /* mark size class on the span */
        pageNode->sizeClass = size;
        pageNode->maxObjectsOnSpan = objectCount;

        /* split span to object */
        NX_U8 *start = (NX_U8 *)span;
        NX_U32 i;
        for (i = 0; i < objectCount; i++)
        {
            objectNode = (NX_SmallCacheObject *)start;
            NX_ListAddTail(&objectNode->list, &cache->objectFreeList);
            NX_AtomicInc(&cache->objectFreeCount);
            start += size;
        }
    }
    /* get a object from list */
    objectNode = NX_ListFirstEntry(&cache->objectFreeList, NX_SmallCacheObject, list);
    NX_ListDel(&objectNode->list); /* del from free list */
    NX_AtomicDec(&cache->objectFreeCount);
    
    return (void *)objectNode;
}

NX_PUBLIC void *NX_HeapAlloc(NX_USize size)
{
    if (!size)
    {
        return NX_NULL;
    }
    NX_MutexLock(&HeapCacheLock, NX_True);
    void *ptr = DoHeapAlloc(size);
    NX_MutexUnlock(&HeapCacheLock);
    return ptr;
}

NX_PRIVATE NX_Error DoHeapFree(void *object)
{
    if (object == NX_NULL) /* can't free NX_NULL object */
    {
        return NX_EINVAL;
    }
    
    /* object to page, then to span */
    void *page = (void *)(((NX_Addr) object) & NX_PAGE_UMASK);
    void *span = NX_PageToSpan(page);
    NX_USize pageCount = NX_PageToSpanCount(page);
    NX_USize size = pageCount * NX_PAGE_SIZE;

    /* according size to free */
    if (size > MAX_SMALL_OBJECT_SIZE)   /* free to big span */
    {
        if (size > MAX_MIDDLE_OBJECT_SIZE) /* free directly to span */
        {
            return NX_PageCacheFree(span);
        }
        else    /* free to middle cache */
        {
            /* if len is too long, free to page cache */
            if (NX_AtomicGet(&MiddleSizeCache.spanFreeCount) + 1 >= MAX_MIDDLE_OBJECT_THRESOLD) 
            {
                return NX_PageCacheFree(span);
            }
            else    /* free to span free list */
            {
                NX_PageSpan *spanNode = (NX_PageSpan *) span;
                NX_ListAdd(&spanNode->list, &MiddleSizeCache.spanFreeList);
                NX_AtomicInc(&MiddleSizeCache.spanFreeCount);
            }
            return NX_EOK;
        }
    }
    else    /* free to small cache */
    {
        /* NOTICE: must get page node from span page */
        NX_Page *pageNode = NX_PageFromPtr(NX_PageZoneGetBuddySystem(NX_PAGE_ZONE_NORMAL), span);
        NX_ASSERT(pageNode != NX_NULL);

        /* get class size from page */
        NX_USize sizeClass = pageNode->sizeClass;
        /* get left objects */
        NX_HeapCache *cache = SizeToCache(sizeClass);
 
        /* if objects in span is full, free all objects */
        if (NX_AtomicGet(&cache->objectFreeCount) + 1 >= pageNode->maxObjectsOnSpan)
        {
            /* empty object free list */
            NX_ListInit(&cache->objectFreeList);
            NX_AtomicSet(&cache->objectFreeCount, 0);

            /* free span to page cache */
            if (NX_AtomicGet(&cache->spanFreeCount) + 1 >= MAX_SMALL_SPAN_THRESOLD)
            {
                return NX_PageCacheFree(span);
            }
            else    /* add span to span list */
            {
                NX_PageSpan *spanNode = (NX_PageSpan *) span;
                NX_ListAdd(&spanNode->list, &cache->spanFreeList);
                NX_AtomicInc(&cache->spanFreeCount);
            }
        }
        else    /* free to object free list */
        {
            NX_SmallCacheObject *objectNode = (NX_SmallCacheObject *)object;
            NX_ListAdd(&objectNode->list, &cache->objectFreeList);
            NX_AtomicInc(&cache->objectFreeCount);
        }
    }
    return NX_EOK;
}

NX_PUBLIC NX_Error NX_HeapFree(void *object)
{
    if (object == NX_NULL) /* can't free NX_NULL object */
    {
        return NX_EINVAL;
    }
    NX_MutexLock(&HeapCacheLock, NX_True);
    NX_Error err = DoHeapFree(object);
    NX_MutexUnlock(&HeapCacheLock);
    return err;
}

NX_PUBLIC NX_USize NX_HeapGetObjectSize(void *object)
{
    if (object == NX_NULL) /* can't free NX_NULL object */
    {
        return 0;
    }
    NX_MutexLock(&HeapCacheLock, NX_True);
    /* object to page, then to span */
    void *page = (void *)(((NX_Addr) object) & NX_PAGE_UMASK);
    void *span = NX_PageToSpan(page);
    /* NOTICE: page must be span page */
    NX_Page *pageNode = NX_PageFromPtr(NX_PageZoneGetBuddySystem(NX_PAGE_ZONE_NORMAL), span);
    NX_ASSERT(pageNode != NX_NULL);
    NX_MutexUnlock(&HeapCacheLock);
    /* get class size from page */
    return pageNode->sizeClass;
}

NX_PUBLIC void NX_HeapCacheInit(void)
{
    HeapSizeClassInit();
    NX_MutexInit(&HeapCacheLock);
}
