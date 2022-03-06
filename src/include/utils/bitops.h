/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Bit oprate utils
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-31     JasonHu           Init
 */

#ifndef __UTILS_BITOPS__
#define __UTILS_BITOPS__

#include <xbook.h>

NX_INLINE int NX_FLS(int x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u)) {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u)) {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u)) {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u)) {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u)) {
        x <<= 1;
        r -= 1;
    }
    return r;
}

#define NX_FFS(x) ({ NX_U32 __t = (x); NX_FLS(__t & -__t); })

#endif  /* __UTILS_BITOPS__ */
