/**
 * Copyright (c) 2018-2021, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: string tools 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-3      JasonHu           Init
 */

#include <utils/string.h>
#include <utils/log.h>

NX_PUBLIC char *NX_StrCopy(const char *dst, const char *src)
{
    if (dst == NX_NULL || src == NX_NULL)
    {
        NX_LOG_D("NX_NULL arg: %s", __func__);
        return NX_NULL;
    }
    char *dstPtr = (char *) dst;
    while ((*dstPtr++ = *src++))
    {
    }
    return (char *)dst;
}

NX_PUBLIC char *NX_StrCopyN(const char *dst, const char *src, NX_USize len)
{
    if (dst == NX_NULL || src == NX_NULL || !len)
    {
        NX_LOG_D("NX_NULL arg: %s", __func__);
        return NX_NULL;
    }
    char *dstPtr = (char *) dst;
    while ((*dstPtr++ = *src++) && len-- > 0)
    {
    }
    return (char *)dst;
}

NX_PUBLIC int NX_StrCmp(const char *a, const char *b)
{
    if (a == NX_NULL || b == NX_NULL)
    {
        NX_LOG_D("NX_NULL arg: %s", __func__);
        return 0;
    }

    while (*a && *a == *b)
    {
        a++;
        b++;
    }

    return (*a - *b);
}

NX_PUBLIC int NX_StrLen(const char *str)
{
    if (str == NX_NULL)
    {
        NX_LOG_D("NX_NULL arg: %s", __func__);
        return 0;
    }
    const char *p = str;
    while(*p++);
    return (p - str - 1);
}
