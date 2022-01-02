/**
 * Copyright (c) 2018-2021, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Thread context 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-16     JasonHu           Init
 */

#include <Sched/Context.h>
#include <Utils/Memory.h>
#include <Utils/Log.h>
#include <Context.h>
#include <Regs.h>

IMPORT void HAL_ContextSwitchNext(Addr nextSP);
IMPORT void HAL_ContextSwitchPrevNext(Addr prevSP, Addr nextSP);

PRIVATE void *HAL_ContextInit(void *startEntry, void *exitEntry, void *arg, void *stackTop)
{
    U8 *stack = NULL;
    HAL_Context *context = NULL;

    stack = (U8 *)stackTop;
    stack = (U8 *)ALIGN_DOWN((Uint)stack, sizeof(Uint));

    stack -= sizeof(HAL_Context);
    context = (HAL_Context *)stack;
    
    Zero(context, sizeof(HAL_Context));

    context->a0 = (Uint)arg;
    context->epc = (Uint)startEntry;
    context->sp = (Uint)(((Uint)stack) + sizeof(HAL_Context));
    context->ra = (Uint)exitEntry;
    
    /**
     * allow to access user space memory,
     * return to supervisor mode,
     * enable interrupt
     */
    context->sstatus = SSTATUS_SUM | SSTATUS_SPP | SSTATUS_SPIE;

    return (void *)stack;
}

INTERFACE struct ContextOps ContextOpsInterface = 
{
    .init           = HAL_ContextInit,
    .switchNext     = HAL_ContextSwitchNext,
    .switchPrevNext = HAL_ContextSwitchPrevNext,
};
