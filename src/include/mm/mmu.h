/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Memory Manage Unite 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-2-1       JasonHu           Init
 */

#ifndef __MM_MMU__
#define __MM_MMU__

#include <xbook.h>
#include <arch/mmu.h>

struct NX_Mmu
{
    void *table;    /* mmu table */
    NX_Addr virStart; /* vir addr start */
    NX_Addr virEnd;   /* vir addr end */
    NX_Addr earlyEnd; /* early map end(only for kernel self map) */
};
typedef struct NX_Mmu NX_Mmu;

struct NX_MmuOps
{
    void (*setPageTable)(NX_Addr addr);
    NX_Addr (*getPageTable)(void);
    void (*enable)(void);

    void *(*mapPage)(NX_Mmu *mmu, NX_Addr virAddr, NX_Size size, NX_UArch attr);
    void *(*mapPageWithPhy)(NX_Mmu *mmu, NX_Addr virAddr, NX_Addr phyAddr, NX_Size size, NX_UArch attr);
    NX_Error (*unmapPage)(NX_Mmu *mmu, NX_Addr virAddr, NX_Size size);
    void *(*vir2Phy)(NX_Mmu *mmu, NX_Addr virAddr);
};

NX_INTERFACE NX_IMPORT struct NX_MmuOps NX_MmuOpsInterface; 

#define NX_MmuSetPageTable(addr)                    NX_MmuOpsInterface.setPageTable(addr)
#define NX_MmuGetPageTable()                        NX_MmuOpsInterface.getPageTable()
#define NX_MmuEnable()                              NX_MmuOpsInterface.enable()
#define NX_MmuMapPage(mmu, virAddr, size, attr)     NX_MmuOpsInterface.mapPage(mmu, virAddr, size, attr)
#define NX_MmuMapPageWithPhy(mmu, virAddr, phyAddr, size, attr) \
        NX_MmuOpsInterface.mapPageWithPhy(mmu, virAddr, phyAddr, size, attr)
#define NX_MmuUnmapPage(mmu, virAddr, size)         NX_MmuOpsInterface.unmapPage(mmu, virAddr, size)
#define NX_MmuVir2Phy(mmu, virAddr)                 NX_MmuOpsInterface.vir2Phy(mmu, virAddr)

void NX_MmuInit(NX_Mmu *mmu, void *pageTable, NX_Addr virStart, NX_Size size, NX_Addr earlyEnd);

#endif /* __MM_MMU__ */
