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

struct NX_HeapCache
{
    NX_List spanFreeList;       /* free list for middle objects, 
                                   the span object can also be split into small objects */
    NX_List objectFreeList;     /* free list for small objects */
    NX_USize classSize;         /* heap cache size */
    NX_Atomic spanFreeCount;    /* counts for middle objects */
    NX_Atomic objectFreeCount;  /* counts for small objects */
    NX_Mutex lock;              /* lock for cache list */
};
typedef struct NX_HeapCache NX_HeapCache;

struct NX_SizeClass
{
    NX_USize size;
    struct NX_HeapCache cache;
};

/* small object struct */
struct NX_SmallCacheObject
{
    NX_List list;
};
typedef struct NX_SmallCacheObject NX_SmallCacheObject;

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
