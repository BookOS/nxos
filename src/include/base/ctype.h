/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: ctype for nxos
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-3-21      JasonHu           Init
 */

#ifndef __UTILS_CTYPE__
#define __UTILS_CTYPE__

#include <nxos.h>

int NX_IsDigit(int c);
int NX_IsSpace(int c);
int NX_IsUpper(int c);
int NX_IsAlpha(int c);

#endif  /* __UTILS_CTYPE__ */
