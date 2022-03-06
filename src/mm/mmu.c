/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Memory Manage Unite 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-2-2       JasonHu           Init
 */

#include <mm/mmu.h>
#include <mm/page.h>
#include <utils/memory.h>
#include <xbook/debug.h>

void NX_MmuInit(NX_Mmu *mmu, void *pageTable, NX_Addr virStart, NX_Size size, NX_Addr earlyEnd)
{
    mmu->table = pageTable;
    mmu->virStart = virStart & NX_PAGE_ADDR_MASK;
    mmu->virEnd = virStart + NX_PAGE_ALIGNUP(size);
    mmu->earlyEnd = earlyEnd & NX_PAGE_ADDR_MASK;
}
