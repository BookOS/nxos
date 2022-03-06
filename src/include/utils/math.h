/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Math utils
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-25     JasonHu           Init
 */

#ifndef __UTILS_MATH__
#define __UTILS_MATH__

#include <xbook.h>

NX_INLINE int NX_PowInt(int x, int n)
{
    int res = 1;
    if (n < 0)
    {
        x = 1 / x;
        n = -n;
    }
    while (n)
    {
        if(n & 1)
        {
            res *= x;
        }
        x *= x;
        n >>= 1;
    }
    return res;
}

#endif  /* __UTILS_MATH__ */