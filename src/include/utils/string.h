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

char *NX_StrCopy(const char *dst, const char *src);
int NX_StrCmp(const char *a, const char *b);
int NX_StrLen(const char *str);
char *NX_StrCopyN(const char *dst, const char *src, NX_Size len);

int NX_VSNPrintf(char *buf, int bufLen, const char *fmt, NX_VarList args);
int NX_SNPrintf(char *buf, int buflen, const char *fmt, ...);

#endif  /* __UTILS_STRING__ */