/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Console modules header
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-1      JasonHu           Init
 */

#ifndef __MODS_CONSOLE_HEADER__
#define __MODS_CONSOLE_HEADER__

#include <xbook.h>

#define NX_CON_NEWLINE "\n"

#define NX_Endln NX_CON_NEWLINE

void NX_ConsoleOutChar(char ch);
void NX_ConsoleOutStr(const char *str);

void NX_Printf(const char *fmt, ...);

#endif  /* __MODS_CONSOLE_HEADER__ */