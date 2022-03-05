/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Console output
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-1      JasonHu           Init
 */

#include <utils/memory.h>
#include <utils/log.h>
#include <utils/string.h>
#include <utils/var_args.h>

#ifdef CONFIG_PRINT_BUF_LEN
#define NX_PRINT_BUF_LEN CONFIG_PRINT_BUF_LEN
#else
#define NX_PRINT_BUF_LEN 256
#endif

/* Conosle HAL */
NX_INTERFACE NX_WEAK_SYM void HAL_ConsoleOutChar(char ch) {}

void NX_ConsoleOutChar(char ch)
{
    HAL_ConsoleOutChar(ch);
}

void NX_ConsoleOutStr(const char *str)
{
    char *p = (char *)str;
    while (*p)
    {
        HAL_ConsoleOutChar(*p++);
    }
}

void NX_Printf(const char *fmt, ...)
{
	char buf[NX_PRINT_BUF_LEN] = {0};
	NX_VarList arg;
    NX_VarStart(arg, fmt);
    NX_VSNPrintf(buf, NX_PRINT_BUF_LEN, fmt, arg);
	NX_VarEnd(arg);
    NX_ConsoleOutStr(buf);
}
