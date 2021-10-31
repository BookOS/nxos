/**
 * Copyright (c) 2018-2021, BookOS Development Team
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

#include <XBook.h>

PUBLIC char *StrCopy(char *dst, char *src);
PUBLIC char StrCmp(const char* a, const char* b);
PUBLIC int StrLen(const char *str);

#endif  /* __UTILS_STRING__ */