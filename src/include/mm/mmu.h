/**
 * Copyright (c) 2018-2022, BookOS Development Team
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

#define NX_MmuSetPageTable          NX_MmuOpsInterface.setPageTable
#define NX_MmuGetPageTable          NX_MmuOpsInterface.getPageTable
#define NX_MmuEnable                NX_MmuOpsInterface.enable

#define NX_MmuMapPage               NX_MmuOpsInterface.mapPage
#define NX_MmuMapPageWithPhy        NX_MmuOpsInterface.mapPageWithPhy
#define NX_MmuUnmapPage             NX_MmuOpsInterface.unmapPage
#define NX_MmuVir2Phy               NX_MmuOpsInterface.vir2Phy

NX_PUBLIC void NX_MmuInit(NX_Mmu *mmu, void *pageTable, NX_Addr virStart, NX_Size size, NX_Addr earlyEnd);

#endif /* __MM_MMU__ */
