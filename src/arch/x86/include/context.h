/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: platform context switch
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-16     JasonHu           Init
 */

#ifndef __PLATFORM_CONTEXT__
#define __PLATFORM_CONTEXT__

#include <xbook.h>

typedef void (*NX_HalThreadHandler)(void *);

struct NX_HalContext
{
    NX_U32 ebp;
    NX_U32 ebx;
    NX_U32 edi;
    NX_U32 esi;

    /* first run point to handler, other time point to the ret addr of switch_to */
    void (*eip) (NX_HalThreadHandler handler, void *arg, void (*texit)());

    NX_U32 unused;
    NX_HalThreadHandler handler;
    void *arg;
    void *exit; /* thread exit call */
};
typedef struct NX_HalContext NX_HalContext;

#endif  /* __PLATFORM_CONTEXT__ */
