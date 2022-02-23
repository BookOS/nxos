/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: String utils
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-3      JasonHu           Init
 */

#ifndef __UTILS_STRING__
#define __UTILS_STRING__

#include <xbook.h>
#include <utils/var_args.h>

NX_PUBLIC char *NX_StrCopy(const char *dst, const char *src);
NX_PUBLIC int NX_StrCmp(const char *a, const char *b);
NX_PUBLIC int NX_StrLen(const char *str);
NX_PUBLIC char *NX_StrCopyN(const char *dst, const char *src, NX_Size len);

NX_PUBLIC int NX_VSNPrintf(char *buf, int bufLen, const char *fmt, NX_VarList args);
NX_PUBLIC int NX_SNPrintf(char *buf, int buflen, const char *fmt, ...);

#endif  /* __UTILS_STRING__ */