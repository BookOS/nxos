/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: memory barrier
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-7      JasonHu           Init
 */

#ifndef __MEMORY_BARRIER__
#define __MEMORY_BARRIER__

#include <nxos.h>

struct NX_MemBarrierOps
{
    void (*barrier)(void);
    void (*barrierRead)(void);
    void (*barrierWrite)(void);
    void (*barrierInstruction)(void);
};

NX_INTERFACE NX_IMPORT struct NX_MemBarrierOps NX_MemBarrierOpsInterface;

#define NX_MemoryBarrier()            NX_MemBarrierOpsInterface.barrier()
#define NX_MemoryBarrierRead()        NX_MemBarrierOpsInterface.barrierRead()
#define NX_MemoryBarrierWrite()       NX_MemBarrierOpsInterface.barrierWrite()
#define NX_MemoryBarrierInstruction() NX_MemBarrierOpsInterface.barrierInstruction()

#endif /* __MEMORY_BARRIER__ */
