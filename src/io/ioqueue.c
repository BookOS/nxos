/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: io queue
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-21      JasonHu           Init
 */

#include <base/ioqueue.h>
#include <base/malloc.h>

NX_Error NX_IoQueueInit(NX_IoQueue * queue, char *buf, NX_Size length)
{
    if (!queue || !buf || !length)
    {
        return NX_EINVAL;
    }

    queue->buf = buf;
    queue->length = length;
    queue->getIndex = 0;
    queue->putIndex = 0;

    NX_SemaphoreInit(&queue->mutex, 1);
    NX_SemaphoreInit(&queue->full, 0);
    NX_SemaphoreInit(&queue->empty, length);
    return NX_EOK;
}

NX_IoQueue *NX_IoQueueCreate(NX_Size length, NX_Error * outErr)
{
    char *buf;
    NX_Error err;

    NX_IoQueue *queue = NX_MemAllocEx(NX_IoQueue);
    if (queue == NX_NULL)
    {
        NX_ErrorSet(outErr, NX_ENOMEM);
        return NX_NULL;
    }

    buf = NX_MemAlloc(length);
    if (buf == NX_NULL)
    {
        NX_ErrorSet(outErr, NX_ENOMEM);
        NX_MemFree(queue);
        return NX_NULL;
    }

    err = NX_IoQueueInit(queue, buf, length);
    if (err != NX_EOK)
    {
        NX_ErrorSet(outErr, err);
        NX_MemFree(buf);
        NX_MemFree(queue);
        return NX_NULL;
    }

    NX_ErrorSet(outErr, err);
    return queue;
}

NX_Error NX_IoQueueDestroy(NX_IoQueue * queue)
{
    if (queue == NX_NULL)
    {
        return NX_EINVAL;
    }
    
    NX_MemFree(queue->buf);
    NX_MemFree(queue);
    return NX_EOK;
}

char NX_IoQueueGet(NX_IoQueue * queue, NX_Error *outErr)
{
    char data;
    if (queue == NX_NULL)
    {
        NX_ErrorSet(outErr, NX_EINVAL);
        return 0;
    }
    
    NX_SemaphoreWait(&queue->full);
    NX_SemaphoreWait(&queue->mutex);
    data = queue->buf[queue->getIndex++];
    queue->getIndex %= queue->length;
    NX_SemaphoreSignal(&queue->mutex);
    NX_SemaphoreSignal(&queue->empty);

    NX_ErrorSet(outErr, NX_EOK);
    return data;
}

char NX_IoQueueTryGet(NX_IoQueue * queue, NX_Error *outErr)
{
    char data;
    if (queue == NX_NULL)
    {
        NX_ErrorSet(outErr, NX_EINVAL);
        return 0;
    }
    
    if (NX_SemaphoreTryWait(&queue->full) != NX_EOK)
    {
        NX_ErrorSet(outErr, NX_EAGAIN);
        return 0;
    }

    NX_SemaphoreWait(&queue->mutex);
    data = queue->buf[queue->getIndex++];
    queue->getIndex %= queue->length;
    NX_SemaphoreSignal(&queue->mutex);
    NX_SemaphoreSignal(&queue->empty);

    NX_ErrorSet(outErr, NX_EOK);
    return data;
}

NX_Error NX_IoQueuePut(NX_IoQueue * queue, char data)
{
    if (queue == NX_NULL)
    {
        return NX_EINVAL;
    }
    
    NX_SemaphoreWait(&queue->empty);
    NX_SemaphoreWait(&queue->mutex);
    queue->buf[queue->putIndex++] = data;
    queue->putIndex %= queue->length;
    NX_SemaphoreSignal(&queue->mutex);
    NX_SemaphoreSignal(&queue->full);

    return NX_EOK;
}

NX_Error NX_IoQueueTryPut(NX_IoQueue * queue, char data)
{
    if (queue == NX_NULL)
    {
        return NX_EINVAL;
    }
    
    if (NX_SemaphoreTryWait(&queue->empty) != NX_EOK)
    {
        return NX_EAGAIN;
    }

    NX_SemaphoreWait(&queue->mutex);
    queue->buf[queue->putIndex++] = data;
    queue->putIndex %= queue->length;
    NX_SemaphoreSignal(&queue->mutex);
    NX_SemaphoreSignal(&queue->full);

    return NX_EOK;
}
