/**
 * Copyright (c) 2018-2022, NXOS Development Team
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

#include <nxos.h>
#include <base/atomic.h>
#include <base/list.h>
#include <base/mutex.h>
#include <base/page_cache.h>

struct NX_HeapCache
{
    NX_List objectFreeList;
    NX_Size classSize;         /* heap cache size */
    NX_Size objectFreeCount;
    NX_Mutex lock;              /* lock for cache list */
};
typedef struct NX_HeapCache NX_HeapCache;

struct NX_HeapSizeClass
{
    NX_Size size;
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
    NX_Size objectFreeCount;   /* counts for small objects */
    NX_Size maxObjects;        /* max objects on this cache system */
};
typedef struct NX_HeapSmallCacheSystem NX_HeapSmallCacheSystem;

void NX_HeapCacheInit(void);

void *NX_HeapAlloc(NX_Size size);
NX_Error NX_HeapFree(void *object);
NX_Size NX_HeapGetObjectSize(void *object);

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
