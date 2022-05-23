/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Log utils
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-31     JasonHu           Init
 */

#ifndef __UTILS_LOG__
#define __UTILS_LOG__

#include <nxos.h>

#ifdef CONFIG_NX_DEBUG
#define NX_LOG_ENABLE
#endif

#ifdef CONFIG_NX_DEBUG_COLOR
#define NX_LOG_COLOR
#endif

/* log level */
#define NX_LOG_ERROR   0
#define NX_LOG_WARNING 1
#define NX_LOG_INFO    2
#define NX_LOG_DBG     3

#ifdef NX_LOG_NAME
#ifndef NX_LOG_MOD_NAME
#define NX_LOG_MOD_NAME NX_LOG_NAME
#endif
#else
#ifndef NX_LOG_MOD_NAME
#define NX_LOG_MOD_NAME "LOG"
#endif
#endif /* NX_LOG_NAME */

#ifdef NX_LOG_ENABLE
#include <base/console.h>
#include <base/clock.h>

#ifdef NX_LOG_LEVEL
#ifndef NX_LOG_MOD_LEVEL
#define NX_LOG_MOD_LEVEL NX_LOG_LEVEL
#endif
#else
#ifdef CONFIG_NX_LOG_LEVEL
#define NX_LOG_MOD_LEVEL CONFIG_NX_LOG_LEVEL
#else
#define NX_LOG_MOD_LEVEL NX_LOG_WARNING
#endif /* CONFIG_NX_LOG_LEVEL */
#endif /* NX_LOG_LEVEL */

#ifdef CONFIG_NX_DEBUG_LINENUMBER
#define __NX_LOG_BEGIN_FMT "[%s-%s] (%s:%d) "
#define __NX_LOG_LINENUMBER_ARG , __func__, __LINE__
#else
#define __NX_LOG_BEGIN_FMT "[%s-%s] "
#define __NX_LOG_LINENUMBER_ARG
#endif

/**
 * terminal color
 * BLACK    30
 * RED      31
 * GREEN    32
 * YELLOW   33
 * BLUE     34
 * PURPLE   35
 * CYAN     36
 * WHITE    37
 */
#ifdef NX_LOG_COLOR
#define __NX_LOG_COLOR(n) NX_Printf("\033[%sm", #n)
#define __NX_LOG_BEGIN(logName, color) NX_Printf("\033[%sm" __NX_LOG_BEGIN_FMT, #color, logName, NX_LOG_MOD_NAME __NX_LOG_LINENUMBER_ARG)
#define __NX_LOG_END NX_Printf("\033[0m" NX_CON_NEWLINE)
#else
#define __NX_LOG_COLOR(n)
#define __NX_LOG_BEGIN(logName, color) NX_Printf(__NX_LOG_BEGIN_FMT, logName, NX_LOG_MOD_NAME, __func__, __LINE__ __NX_LOG_LINENUMBER_ARG)
#define __NX_LOG_END NX_Printf(NX_CON_NEWLINE)
#endif /* NX_LOG_COLOR */


#ifdef CONFIG_NX_DEBUG_TIMELINE
#define NX_LOG_TIMELINE NX_Printf("[%6d.%03d] ", NX_ClockTickGetMillisecond() / 1000, NX_ClockTickGetMillisecond() % 1000);
#else
#define NX_LOG_TIMELINE
#endif

NX_Error LogLineLock(NX_UArch *level);
NX_Error LogLineUnlock(NX_UArch level);

#define NX_LOG_LINE(logName, color, ...) \
    do \
    { \
        NX_UArch _level; \
        LogLineLock(&_level); \
        NX_LOG_TIMELINE \
        __NX_LOG_BEGIN(logName, color); \
        NX_Printf(__VA_ARGS__); \
        __NX_LOG_END; \
        LogLineUnlock(_level); \
    } \
    while (0)
#define __NX_LOG_RAW(...) NX_LOG_TIMELINE \
    do \
    { \
        NX_UArch _level; \
        LogLineLock(&_level); \
        NX_Printf(__VA_ARGS__); \
        LogLineUnlock(_level); \
    } \
    while (0)

#else
#define NX_LOG_LINE(logName, color, fmt, ...)
#define __NX_LOG_RAW(fmt, ...)
#endif /* NX_LOG_ENABLE */

/**
 * Log api
 */
#if (NX_LOG_MOD_LEVEL >= NX_LOG_DBG)
#define NX_LOG_D(...)      NX_LOG_LINE("DBG", 0, __VA_ARGS__)
#else
#define NX_LOG_D(...)
#endif

#if (NX_LOG_MOD_LEVEL >= NX_LOG_INFO)
#define NX_LOG_I(...)      NX_LOG_LINE("INFO", 32, __VA_ARGS__)
#else
#define NX_LOG_I(...)
#endif

#if (NX_LOG_MOD_LEVEL >= NX_LOG_WARNING)
#define NX_LOG_W(...)      NX_LOG_LINE("WARN", 33, __VA_ARGS__)
#else
#define NX_LOG_W(...)
#endif

#if (NX_LOG_MOD_LEVEL >= NX_LOG_ERROR)
#define NX_LOG_E(...)      NX_LOG_LINE("ERR", 31, __VA_ARGS__)
#else
#define NX_LOG_E(...)
#endif

#define NX_LOG_RAW(...)         __NX_LOG_RAW(__VA_ARGS__)

#endif  /* __UTILS_LOG__ */
