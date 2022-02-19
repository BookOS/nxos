/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Heap cache
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-25     JasonHu           Init
 */

#ifndef __MM_HEAP_CACHE__
#define __MM_HEAP_CACHE__

#include <xbook.h>
#include <xbook/atomic.h>
#include <utils/list.h>
#include <sched/mutex.h>
#include <mm/page_cache.h>

struct NX_HeapCache
{
    NX_List objectFreeList;
    NX_USize classSize;         /* heap cache size */
    NX_USize objectFreeCount;
    NX_Mutex lock;              /* lock for cache list */
};
typedef struct NX_HeapCache NX_HeapCache;

struct NX_HeapSizeClass
{
    NX_USize size;
    struct NX_HeapCache cache;
};

/* small cache object */
struct NX_HeapSmallCacheObject
{
    NX_List list;
};
typedef struct NX_HeapSmallCacheObject NX_HeapSmallCacheObject;

/* small cache system */
struct NX_HeapSmallCacheSystem
{
    NX_PageSpan pageSpan;
    NX_List objectFreeList;     /* free list for small objects */
    NX_USize objectFreeCount;   /* counts for small objects */
    NX_USize maxObjects;        /* max objects on this cache system */
};
typedef struct NX_HeapSmallCacheSystem NX_HeapSmallCacheSystem;

NX_PUBLIC void NX_HeapCacheInit(void);

NX_PUBLIC void *NX_HeapAlloc(NX_USize size);
NX_PUBLIC NX_Error NX_HeapFree(void *object);
NX_PUBLIC NX_USize NX_HeapGetObjectSize(void *object);

NX_INLINE NX_Error __HeapFreeSatety(void **object)
{
    NX_Error err = NX_HeapFree(*object);
    if (err == NX_EOK)
    {
        *object = NX_NULL;
    }
    return err;
}

#define NX_HeapFreeSatety(p) __HeapFreeSatety((void **)(&(p)))

#endif /* __MM_HEAP_CACHE__ */
