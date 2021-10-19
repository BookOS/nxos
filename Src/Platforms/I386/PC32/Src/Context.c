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

#include <Context.h>
#include <HAL.h>
#include <Interrupt.h>
#include <Mods/Console/Console.h>

/**
 * any thread will come here when first start
 */
PRIVATE void ThreadEntry(HAL_ThreadHandler handler, void *arg, void (*texit)())
{
    HAL_InterruptEnable();
    handler(arg);
    if (texit)
        texit();
    PANIC("Thread execute done, should never be here!" Endln);
}

INTERFACE U8 *HAL_ContextInit(void *entry, void *arg, U8 *stackBase, void *exit)
{
    U8 *stack;
    stack = stackBase + sizeof(Uint);
    stack = (U8 *)ALIGN_DOWN((Uint)stack, sizeof(Uint));
    stack -= sizeof(HAL_TrapFrame);
    stack -= sizeof(HAL_Context);

    HAL_Context *context = (HAL_Context *)stack;
    context->eip = ThreadEntry;
    context->handler = entry;
    context->arg = arg;
    context->exit = exit;
    context->ebp = context->ebx = context->esi = context->edi = 0;
    return stack;
}
