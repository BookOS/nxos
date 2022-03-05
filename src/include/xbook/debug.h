/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Debug system
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-31     JasonHu           Init
 * 2022-1-3       JasonHu           Move to xbook dir
 */

#ifndef __XBOOK_NX_DEBUG__
#define __XBOOK_NX_DEBUG__

#include <xbook.h>

#ifdef CONFIG_NX_DEBUG

void NX_DebugAssertionFailure(char *exp, char *file, char *baseFile, int line);

#define NX_ASSERT(exp) \
    do \
    { \
        if (!(exp)) \
        { \
            NX_DebugAssertionFailure(#exp, __FILE__, __BASE_FILE__, __LINE__); \
        } \
    } while(0)
#else
#define NX_ASSERT(exp)
#endif

/**
 * Spin here
 */
void NX_DebugSpin(const char *str);
#define NX_SPIN(str) NX_DebugSpin(str)

/**
 * OS Panic
 */
void NX_DebugPanic(const char *str);
#define NX_PANIC(str) NX_DebugPanic(str)

#endif  /* __XBOOK_NX_DEBUG__ */
