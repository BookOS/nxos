/**
 * Copyright (c) 2018-2021, BookOS Development Team
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

#include <XBook.h>
#include <HAL.h>

#define CON_NEWLINE "\n"

#define Endln CON_NEWLINE

#define MAX_INT_BUF_SZ  (64 + 1)
#define MAX_BUF_NR  64

PUBLIC void ConsoleOutChar(char ch);
PUBLIC void ConsoleOutStr(const char *str);
PUBLIC char *NumberToString(long n, int radix, int small, int pad, char padChar);

/**
 * Console Output
 */
#define $(v) , (char *)v, 
#define $s(v) $(v)
#define $d(v) , NumberToString((I32)(v), 10, 0, 0, ' '),
#define $x(v) , NumberToString((U32)(v), 16, 1, 0, ' '),
#define $X(v) , NumberToString((U32)(v), 16, 0, 0, ' '),
#define $p(v) , NumberToString((U32)(v), 16, 1, 0, ' '),
#define $P(v) , NumberToString((U32)(v), 16, 0, 0, ' '),
#define $b(v) , NumberToString((U32)(v), 2, 0, 0, ' '),
#define $o(v) , NumberToString((U32)(v), 8, 0, 0, ' '),

#define $d_(v, pad, ch) , NumberToString((I32)(v), 10, 0, pad, ch),
#define $x_(v, pad, ch) , NumberToString((U32)(v), 16, 1, pad, ch),
#define $X_(v, pad, ch) , NumberToString((U32)(v), 16, 0, pad, ch),
#define $p_(v, pad, ch) , NumberToString((U32)(v), 16, 1, pad, ch),
#define $P_(v, pad, ch) , NumberToString((U32)(v), 16, 0, pad, ch),
#define $b_(v, pad, ch) , NumberToString((U32)(v), 2, 0, pad, ch),
#define $o_(v, pad, ch) , NumberToString((U32)(v), 8, 0, pad, ch),

#define Cout(x, ...) \
        do { \
            char *args[] = {x, ##__VA_ARGS__}; \
            int i; \
            for (i = 0; i < sizeof(args)/sizeof(args[0]); i++) \
            { \
                ConsoleOutStr(args[i]); \
            } \
        } while (0)

#endif  /* __MODS_CONSOLE_HEADER__ */