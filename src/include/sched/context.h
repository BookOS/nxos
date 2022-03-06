/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: context switch
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-2       JasonHu           Init
 */

#ifndef __SCHED_CONTEXT___
#define __SCHED_CONTEXT___

#include <xbook.h>

struct NX_ContextOps
{
    void *(*init)(void *startEntry, void *exitEntry, void *arg, void *stackTop);
    void (*switchNext)(NX_Addr nextSP);
    void (*switchPrevNext)(NX_Addr prevSP, NX_Addr nextSP);
};
NX_INTERFACE NX_IMPORT struct NX_ContextOps NX_ContextOpsInterface;

#define NX_ContextInit(startEntry, exitEntry, arg, stackTop)   NX_ContextOpsInterface.init(startEntry, exitEntry, arg, stackTop)
#define NX_ContextSwitchNext(nextSP)                           NX_ContextOpsInterface.switchNext(nextSP)
#define NX_ContextSwitchPrevNext(prevSP, nextSP)               NX_ContextOpsInterface.switchPrevNext(prevSP, nextSP)

#endif /* __SCHED_CONTEXT___ */
