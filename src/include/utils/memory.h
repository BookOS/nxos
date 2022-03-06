/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Memory utils
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-3      JasonHu           Init
 */

#ifndef __UTILS_MEMORY__
#define __UTILS_MEMORY__

#include <xbook.h>

void *NX_MemSet(void *dst, NX_U8 value, NX_Size sz);
void NX_MemCopy(void *dst, void *src, NX_Size sz);
void *NX_MemZero(void *dst, NX_Size sz);
int NX_CompareN(const void *s1, const void *s2, NX_Size nBytes);

#endif  /* __UTILS_MEMORY__ */