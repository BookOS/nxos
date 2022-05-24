/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Console output
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-1      JasonHu           Init
 */

#include <base/memory.h>
#include <base/log.h>
#include <base/string.h>
#include <base/var_args.h>
#include <base/console.h>

#ifdef CONFIG_PRINT_BUF_LEN
#define NX_PRINT_BUF_LEN CONFIG_PRINT_BUF_LEN
#else
#define NX_PRINT_BUF_LEN 256
#endif

NX_PRIVATE void NX_ConsoleOutStr(const char *str)
{
    char *p = (char *)str;
    while (*p)
    {
        NX_ConsoleSendData(*p++);
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
