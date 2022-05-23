/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Page cache base on phy page
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-24     JasonHu           Init
 */

#ifndef __MM_PAGE_CACHE__
#define __MM_PAGE_CACHE__

#include <base/list.h>
#include <base/atomic.h>

/* Combine multiple pages into a span  */
struct NX_PageSpan
{
    NX_List list;  /* NOTICE: list must be first member here */
    NX_Size pageCount; /* page count for this span */
};
typedef struct NX_PageSpan NX_PageSpan;

void NX_PageCacheInit(void);
void *NX_PageCacheAlloc(NX_Size count);
NX_Error NX_PageCacheFree(void *page);

void *NX_PageToSpan(void *page);
NX_Size NX_SpanToCount(void *span);

#endif /* __MM_PAGE_CACHE__ */
