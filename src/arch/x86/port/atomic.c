/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: NX_Atomic for x86 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-11     JasonHu           Init
 */

#include <base/atomic.h>

#define LOCK_PREFIX "lock "

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
    NX_CASM(LOCK_PREFIX "addl %1,%0"   
         : "+m" (atomic->value)   
         : "ir" (value));
}

NX_PRIVATE void NX_HalAtomicSub(NX_Atomic *atomic, long value)
{
    NX_CASM(LOCK_PREFIX "subl %1,%0"   
         : "+m" (atomic->value)   
         : "ir" (value));
}

NX_PRIVATE void NX_HalAtomicInc(NX_Atomic *atomic)
{
    NX_CASM(LOCK_PREFIX "incl %0"   
         : "+m" (atomic->value));
}

NX_PRIVATE void NX_HalAtomicDec(NX_Atomic *atomic)
{
    NX_CASM(LOCK_PREFIX "decl %0"   
         : "+m" (atomic->value));   
}

NX_PRIVATE void NX_HalAtomicSetMask(NX_Atomic *atomic, long mask)
{
    NX_CASM(LOCK_PREFIX "orl %0,%1"
        : : "r" (mask), "m" (*(&atomic->value)) : "memory");
}

NX_PRIVATE void NX_HalAtomicClearMask(NX_Atomic *atomic, long mask)
{    
    NX_CASM(LOCK_PREFIX "andl %0,%1"
         : : "r" (~(mask)), "m" (*(&atomic->value)) : "memory");
}

NX_PRIVATE long NX_HalAtomicSwap(NX_Atomic *atomic, long newValue)
{
    NX_CASM("xchgl %k0,%1"   
         : "=r" (newValue)
         : "m" (*(&atomic->value)), "0" (newValue)   
         : "memory");
    return newValue;
}

NX_PRIVATE long NX_HalAtomicCAS(NX_Atomic *atomic, long old, long newValue)
{
    long prev;
    NX_CASM(LOCK_PREFIX "cmpxchgl %k1,%2"   
         : "=a"(prev)
         : "r"(newValue), "m"(*(&atomic->value)), "0"(old)   
         : "memory");
    return prev;
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
