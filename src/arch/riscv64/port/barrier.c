/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: HAL Memory Barrier
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-9      JasonHu           Init
 */

#include <mm/barrier.h>

NX_PRIVATE void NX_HalMemBarrier(void)
{
    __sync_synchronize();
}

NX_PRIVATE void NX_HalMemBarrierRead(void)
{
    NX_CASM("fence":::"memory");
}

NX_PRIVATE void NX_HalMemBarrierWrite(void)
{
    NX_CASM("fence":::"memory");
}

NX_PRIVATE void NX_HalMemBarrierInstruction(void)
{
    NX_CASM("fence.i":::"memory");
}

NX_INTERFACE struct NX_MemBarrierOps NX_MemBarrierOpsInterface = 
{
    .barrier            = NX_HalMemBarrier,
    .barrierRead        = NX_HalMemBarrierRead,
    .barrierWrite       = NX_HalMemBarrierWrite,
    .barrierInstruction = NX_HalMemBarrierInstruction,
};
