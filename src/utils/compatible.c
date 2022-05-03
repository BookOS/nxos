/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Compatiable functions
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-3      JasonHu           Init
 * 2021-1-6       JasonHu           move to compatible
 * 
 */

#include <xbook.h>

/**
 * compatiable for gcc compiler with optimize
 */
void *memset(void *dst, NX_U8 value, NX_Size sz)
{
    NX_U8 *dstPtr = (NX_U8 *)dst;
	while (sz > 0)
    {
		*dstPtr++ = value;
		--sz;
	}
	return dst;
}

void *memcpy(void *dst, const void *src, NX_Size sz)
{
    NX_U8 *dstPtr = (NX_U8 *)dst;
    NX_U8 *srcPtr = (NX_U8 *)src;
    while (sz-- > 0)
    {
        *dstPtr++ = *srcPtr++;
    }
	return dst;
}
