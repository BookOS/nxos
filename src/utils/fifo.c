/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: fifo buffer
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-06-05     JasonHu           Init
 */

#include <base/fifo.h>
#include <base/malloc.h>
#include <base/math.h>
#include <base/debug.h>
#include <base/barrier.h>
#include <base/memory.h>

NX_Error NX_FifoInit(NX_Fifo * fifo, NX_U8 * buffer, NX_Size size)
{
    if (!NX_IsPowerOf2(size))
    {
        return NX_EINVAL;
    }
    
    fifo->buffer = buffer;
    fifo->size = size;
    fifo->in = fifo->out = 0;
    NX_SpinInit(&fifo->lock);
    return NX_EOK;
}

NX_Fifo * NX_FifoCreate(NX_Size size)
{
    NX_U8 * buffer;
    NX_Fifo * fifo;

    if (!NX_IsPowerOf2(size))
    {
        NX_ASSERT(size < 0x80000000);
        size = NX_RoundupPowOf2(size);
    }

    buffer = NX_MemAlloc(size);
    if (buffer == NX_NULL)
    {
        return NX_NULL;
    }

    fifo = NX_MemAlloc(sizeof(NX_Fifo));
    if (fifo == NX_NULL)
    {
        NX_MemFree(buffer);
        return NX_NULL;
    }
    
    if (NX_FifoInit(fifo, buffer, size) != NX_EOK)
    {
        NX_MemFree(buffer);
        NX_MemFree(fifo);
        fifo = NX_NULL;
    }
    
    return fifo;
}

void NX_FifoDestroy(NX_Fifo *fifo)
{
    NX_MemFree(fifo->buffer);
    NX_MemFree(fifo);
}

NX_Size NX_FifoPut(NX_Fifo * fifo, const NX_U8 * buffer, NX_Size len)
{
    NX_UArch level;
    NX_Size minLen;
    if (NX_SpinLockIRQ(&fifo->lock, &level) != NX_EOK)
    {
        return 0;
    }

    len = NX_MIN(len, fifo->size - fifo->in + fifo->out);
    NX_MemoryBarrier();
    minLen = NX_MIN(len, fifo->size - (fifo->in & (fifo->size - 1)));
    NX_MemCopy((void *)(fifo->buffer + (fifo->in & (fifo->size - 1))), buffer, minLen);
    NX_MemCopy((void *)fifo->buffer, buffer + minLen, len - minLen);
    NX_MemoryBarrierWrite();
    fifo->in += len;
    
    NX_SpinUnlockIRQ(&fifo->lock, level);
    return len;
}

NX_Size NX_FifoGet(NX_Fifo * fifo, const NX_U8 * buffer, NX_Size len)
{
    NX_UArch level;
    NX_Size minLen;
    if (NX_SpinLockIRQ(&fifo->lock, &level) != NX_EOK)
    {
        return 0;
    }

    len = NX_MIN(len, fifo->in - fifo->out);
    NX_MemoryBarrierRead();
    minLen = NX_MIN(len, fifo->size - (fifo->out & (fifo->size - 1)));
    NX_MemCopy((void *)buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), minLen);
    NX_MemCopy((void *)(buffer + minLen), fifo->buffer, len - minLen);
    NX_MemoryBarrier();
    fifo->out += len;
    
    NX_SpinUnlockIRQ(&fifo->lock, level);
    return len;
}

void NX_FifoReset(NX_Fifo *fifo)
{
    NX_UArch level;
    if (NX_SpinLockIRQ(&fifo->lock, &level) != NX_EOK)
    {
        return;
    }
    fifo->in = fifo->out = 0;
    NX_SpinUnlockIRQ(&fifo->lock, level);
}

NX_Size NX_FifoLen(NX_Fifo *fifo)
{
    NX_UArch level;
    NX_Size len;
    if (NX_SpinLockIRQ(&fifo->lock, &level) != NX_EOK)
    {
        return 0;
    }
    len = fifo->in - fifo->out;
    NX_SpinUnlockIRQ(&fifo->lock, level);
    return len;
}

NX_Size NX_FifoAvaliable(NX_Fifo *fifo)
{
    NX_UArch level;
    NX_Size len;
    if (NX_SpinLockIRQ(&fifo->lock, &level) != NX_EOK)
    {
        return 0;
    }
    len = fifo->size - (fifo->in - fifo->out);
    NX_SpinUnlockIRQ(&fifo->lock, level);
    return len;
}
