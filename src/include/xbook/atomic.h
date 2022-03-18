/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Atomic header
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-11     JasonHu           Init
 * 2022-3-18      JasonHu           update ATOMIC_DEFINE macro
 */

#ifndef __XBOOK_ATOMIC__
#define __XBOOK_ATOMIC__

#include <xbook.h>

struct NX_Atomic
{
    NX_VOLATILE NX_IArch value;
};
typedef struct NX_Atomic NX_Atomic;

#define NX_ATOMIC_INIT_VALUE(val) {val}
#define NX_ATOMIC_DEFINE(name, val) NX_Atomic name = NX_ATOMIC_INIT_VALUE(val);

struct NX_AtomicOps
{
    void (*set)(NX_Atomic *atomic, NX_IArch value);
    NX_IArch (*get)(NX_Atomic *atomic);
    void (*add)(NX_Atomic *atomic, NX_IArch value);
    void (*sub)(NX_Atomic *atomic, NX_IArch value);
    void (*inc)(NX_Atomic *atomic);
    void (*dec)(NX_Atomic *atomic);
    void (*setMask)(NX_Atomic *atomic, NX_IArch mask);
    void (*clearMask)(NX_Atomic *atomic, NX_IArch mask);
    NX_IArch (*swap)(NX_Atomic *atomic, NX_IArch newValue);
    NX_IArch (*cas)(NX_Atomic *atomic, NX_IArch old, NX_IArch newValue);
};

NX_INTERFACE NX_IMPORT struct NX_AtomicOps NX_AtomicOpsInterface;

#define NX_AtomicSet(atomic, value)         NX_AtomicOpsInterface.set(atomic, value)
#define NX_AtomicGet(atomic)                NX_AtomicOpsInterface.get(atomic)
#define NX_AtomicAdd(atomic, value)         NX_AtomicOpsInterface.add(atomic, value)
#define NX_AtomicSub(atomic, value)         NX_AtomicOpsInterface.sub(atomic, value)
#define NX_AtomicInc(atomic)                NX_AtomicOpsInterface.inc(atomic)
#define NX_AtomicDec(atomic)                NX_AtomicOpsInterface.dec(atomic)
#define NX_AtomicSetMask(atomic, mask)      NX_AtomicOpsInterface.setMask(atomic, mask)
#define NX_AtomicClearMask(atomic, mask)    NX_AtomicOpsInterface.clearMask(atomic, mask)
#define NX_AtomicSwap(atomic, newValue)     NX_AtomicOpsInterface.swap(atomic, newValue)
#define NX_AtomicCAS(atomic, old, newValue) NX_AtomicOpsInterface.cas(atomic, old, newValue)

#endif /* __XBOOK_ATOMIC__ */
