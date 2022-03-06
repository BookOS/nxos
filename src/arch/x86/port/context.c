/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: NX_Thread context 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-16     JasonHu           Init
 */

#include <context.h>
#include <sched/context.h>
#include <interrupt.h>
#include <xbook/debug.h>
#include <io/irq.h>

NX_IMPORT void NX_HalContextSwitchNext(NX_Addr nextSP);
NX_IMPORT void NX_HalContextSwitchPrevNext(NX_Addr prevSP, NX_Addr nextSP);

/**
 * any thread will come here when first start
 */
NX_PRIVATE void ThreadEntry(NX_HalThreadHandler handler, void *arg, void (*texit)())
{
    NX_IRQ_Enable();
    handler(arg);
    if (texit)
        texit();
    NX_PANIC("Thread execute done, should never be here!");
}

NX_PRIVATE void *NX_HalContextInit(void *startEntry, void *exitEntry, void *arg, void *stackTop)
{
    NX_U8 *stack = NX_NULL;

    stack = stackTop + sizeof(NX_UArch);
    stack = (NX_U8 *)NX_ALIGN_DOWN((NX_UArch)stack, sizeof(NX_UArch));
    stack -= sizeof(NX_HalTrapFrame);
    stack -= sizeof(NX_HalContext);

    NX_HalContext *context = (NX_HalContext *)stack;
    context->eip = ThreadEntry;
    context->handler = startEntry;
    context->arg = arg;
    context->exit = exitEntry;
    context->ebp = context->ebx = context->esi = context->edi = 0;
    return stack;
}

NX_INTERFACE struct NX_ContextOps NX_ContextOpsInterface = 
{
    .init           = NX_HalContextInit,
    .switchNext     = NX_HalContextSwitchNext,
    .switchPrevNext = NX_HalContextSwitchPrevNext,
};
