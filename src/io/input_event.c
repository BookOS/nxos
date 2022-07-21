/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: input event
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-06-04     JasonHu           Init
 */

#include <drvfw/input_event.h>
#include <base/malloc.h>
#include <base/memory.h>

NX_Error NX_InputEventQueueInit(NX_InputEventQueue * eventQueue, NX_Size queueSize)
{
    if (!eventQueue || !queueSize)
    {
        return NX_EINVAL;
    }
    
    NX_InputEvent * eventBuf = NX_MemAlloc(sizeof(NX_InputEvent) * queueSize);
    if (eventBuf == NX_NULL)
    {
        return NX_ENOMEM;
    }
    NX_MemZero(eventBuf, sizeof(NX_InputEvent) * queueSize);

    eventQueue->eventBuf = eventBuf;
    eventQueue->maxSize = queueSize;
    NX_SpinInit(&eventQueue->lock);
    eventQueue->head = eventQueue->tail = 0;

    return NX_EOK;
}

NX_Error NX_InputEventQueueExit(NX_InputEventQueue * eventQueue)
{
    if (!eventQueue)
    {
        return NX_EINVAL;
    }
    
    if (!eventQueue->eventBuf)
    {
        return NX_EPERM;        
    }

    NX_MemFree(eventQueue->eventBuf);
    eventQueue->eventBuf = NX_NULL;
    
    eventQueue->maxSize = 0;
    NX_SpinInit(&eventQueue->lock);
    eventQueue->head = eventQueue->tail = 0;

    return NX_EOK;
}

NX_Error NX_InputEventQueuePut(NX_InputEventQueue * eventQueue, NX_InputEvent * event)
{
    NX_UArch level;
    NX_Error err;
    if ((err = NX_SpinLockIRQ(&eventQueue->lock, &level)) != NX_EOK)
    {
        return err;
    }

    eventQueue->eventBuf[eventQueue->head++] = *event;
    eventQueue->head &= eventQueue->maxSize - 1;

    NX_SpinUnlockIRQ(&eventQueue->lock, level);
    return NX_EOK;
}

NX_Error NX_InputEventQueueGet(NX_InputEventQueue *eventQueue, NX_InputEvent * event)
{
    NX_UArch level;
    NX_Error err;
    if ((err = NX_SpinLockIRQ(&eventQueue->lock, &level)) != NX_EOK)
    {
        return err;
    }

    if (eventQueue->head == eventQueue->tail)
    {
        NX_SpinUnlockIRQ(&eventQueue->lock, level);
        return NX_EAGAIN;    
    }
    *event = eventQueue->eventBuf[eventQueue->tail++];
    eventQueue->tail &= eventQueue->maxSize - 1;

    NX_SpinUnlockIRQ(&eventQueue->lock, level);
    return NX_EOK;
}

NX_Bool NX_InputEventQueueEmpty(NX_InputEventQueue *eventQueue)
{
    NX_UArch level;
    if (NX_SpinLockIRQ(&eventQueue->lock, &level) != NX_EOK)
    {
        return NX_True;
    }

    if (eventQueue->head == eventQueue->tail)
    {
        NX_SpinUnlockIRQ(&eventQueue->lock, level);
        return NX_True;
    }
    NX_SpinUnlockIRQ(&eventQueue->lock, level);
    return NX_False;
}
