/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: HAL NX_Atomic  
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-1      JasonHu           Init
 */

#include <xbook/atomic.h>

NX_PRIVATE void NX_HalAtomicSet(NX_Atomic *atomic, long value)
{
    atomic->value = value;
}

NX_PRIVATE long NX_HalAtomicGet(NX_Atomic *atomic)
{
    return atomic->value;
}

NX_PRIVATE void NX_HalAtomicAdd(NX_Atomic *atomic, long value)
{
    /* gcc build-in functions */
    __sync_fetch_and_add(&atomic->value, value);
}

NX_PRIVATE void NX_HalAtomicSub(NX_Atomic *atomic, long value)
{
    __sync_fetch_and_sub(&atomic->value, value);
}

NX_PRIVATE void NX_HalAtomicInc(NX_Atomic *atomic)
{
    __sync_fetch_and_add(&atomic->value, 1);
}

NX_PRIVATE void NX_HalAtomicDec(NX_Atomic *atomic)
{
    __sync_fetch_and_sub(&atomic->value, 1);
}

NX_PRIVATE void NX_HalAtomicSetMask(NX_Atomic *atomic, long mask)
{
    __sync_fetch_and_or(&atomic->value, mask);
}

NX_PRIVATE void NX_HalAtomicClearMask(NX_Atomic *atomic, long mask)
{    
    __sync_fetch_and_and(&atomic->value, ~mask);
}

NX_PRIVATE long NX_HalAtomicSwap(NX_Atomic *atomic, long newValue)
{
    return __sync_lock_test_and_set(&((atomic)->value), newValue);
}

NX_PRIVATE long NX_HalAtomicCAS(NX_Atomic *atomic, long old, long newValue)
{
    return __sync_val_compare_and_swap(&atomic->value, old, newValue);
}

NX_INTERFACE struct NX_AtomicOps NX_AtomicOpsInterface = 
{
    .set        = NX_HalAtomicSet,
    .get        = NX_HalAtomicGet,
    .add        = NX_HalAtomicAdd,
    .sub        = NX_HalAtomicSub,
    .inc        = NX_HalAtomicInc,
    .dec        = NX_HalAtomicDec,
    .setMask    = NX_HalAtomicSetMask,
    .clearMask  = NX_HalAtomicClearMask,
    .swap       = NX_HalAtomicSwap,
    .cas        = NX_HalAtomicCAS,
};
