/**
 * Copyright (c) 2018-2022, NXOS Development Team
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

#include <nxos.h>
#include <base/var_args.h>

char *NX_StrCopy(const char *dst, const char *src);
int NX_StrCmp(const char *a, const char *b);
int NX_StrLen(const char *str);
NX_Size NX_StrCopyN(char *dest, const char *src, NX_Size len);

int NX_VSNPrintf(char *buf, int bufLen, const char *fmt, NX_VarList args);
int NX_SNPrintf(char *buf, int buflen, const char *fmt, ...);

int NX_StrCmpN(const char * s1, const char * s2, NX_Size n);

char *NX_StrChr(const char *s, int c);
char *NX_StrChrReverse(const char *s, int c);

unsigned long NX_StrToUL(const char * nptr, char ** endptr, int base);

char * NX_StrCat(char * strDest , const char * strSrc);
char * NX_StrDup(const char *s);

NX_Size NX_StrCatN(char * dest, const char * src, NX_Size n);

#endif  /* __UTILS_STRING__ */