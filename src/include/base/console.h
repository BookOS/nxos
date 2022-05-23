/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Console header
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-1      JasonHu           Init
 * 2022-3-18      JasonHu           move to drivers dir
 */

#ifndef __DRIVERS_CONSOLE_HEADER__
#define __DRIVERS_CONSOLE_HEADER__

#include <nxos.h>

#define NX_CON_NEWLINE "\n"

#define NX_Endln NX_CON_NEWLINE

void NX_ConsoleSendData(char ch);
void NX_ConsoleReceveData(char ch);

void NX_Printf(const char *fmt, ...);

#endif  /* __DRIVERS_CONSOLE_HEADER__ */
