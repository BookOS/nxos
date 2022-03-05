/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Page init 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-28     JasonHu           Init
 */

#include <mm/mmu.h>
#include <mm/page.h>
#include <arch/mmu.h>
#include <page_zone.h>
#include <platform.h>

#include <utils/memory.h>

#define NX_LOG_LEVEL NX_LOG_INFO
#define NX_LOG_NAME "Page"
#include <utils/log.h>

#include <xbook/debug.h>

#define GRUB2_READ_MEMORY_BYTES_ADDR (0x000001000)

NX_Mmu KernelMMU;

NX_PRIVATE NX_U32 KernelTable[NX_PAGE_SIZE / sizeof(NX_U32)] NX_CALIGN(NX_PAGE_SIZE);

NX_PRIVATE void NX_HalEarlyMap(NX_Mmu *mmu, NX_Addr virStart, NX_Size size)
{
    NX_LOG_I("OS map early on [%p~%p]", virStart, virStart + size);
    NX_MmuMapPageWithPhy(&KernelMMU, virStart, virStart, size, NX_PAGE_ATTR_KERNEL);
}

/**
 * Init physic memory and map kernel on virtual memory.
 */
void NX_HalPageZoneInit(void)
{
    NX_Size memSize = *(NX_Size *)GRUB2_READ_MEMORY_BYTES_ADDR;
    
    NX_LOG_I("Memory NX_Size: %x Bytes %d MB", memSize, memSize / NX_MB);

    if (memSize == 0)
    {
        NX_PANIC("Get Memory NX_Size Failed!");
    }
    if (memSize < MEM_MIN_SIZE)
    {
        NX_LOG_E("Must has %d MB memory!", MEM_MIN_SIZE / NX_MB);
        NX_PANIC("Memory too small");
    }
    
    /* calc normal base & size */
    NX_Size normalSize = memSize - MEM_DMA_SIZE - MEM_KERNEL_SZ;
    normalSize /= 2;
    if (normalSize > MEM_KERNEL_SPACE_SZ)
    {
        normalSize = MEM_KERNEL_SPACE_SZ;
    }
    
    /* calc user base & size */
    NX_Addr userBase = MEM_NORMAL_BASE + normalSize;
    NX_Size userSize = memSize - normalSize;

    NX_LOG_I("DMA memory base: %x NX_Size:%d MB", MEM_DMA_BASE, MEM_DMA_SIZE / NX_MB);
    NX_LOG_I("Normal memory base: %x NX_Size:%d MB", MEM_NORMAL_BASE, normalSize / NX_MB);
    NX_LOG_I("User memory base: %x NX_Size:%d MB", userBase, userSize / NX_MB);

    /* init page zone */
    NX_PageInitZone(NX_PAGE_ZONE_DMA, (void *)MEM_DMA_BASE, MEM_DMA_SIZE);
    NX_PageInitZone(NX_PAGE_ZONE_NORMAL, (void *)MEM_NORMAL_BASE, normalSize);
    NX_PageInitZone(NX_PAGE_ZONE_USER, (void *)userBase, userSize);

    NX_MmuInit(&KernelMMU, KernelTable, 0, MEM_KERNEL_TOP, userBase);

    NX_HalEarlyMap(&KernelMMU, KernelMMU.virStart, KernelMMU.earlyEnd - KernelMMU.virStart);

    NX_MmuSetPageTable((NX_UArch)KernelMMU.table);
    NX_MmuEnable();

    NX_LOG_I("MMU enabled");
}

void *NX_HalGetKernelPageTable(void)
{
    return KernelMMU.table;
}

NX_IMPORT NX_Addr __NX_BssStart;
NX_IMPORT NX_Addr __NX_BssEnd;

void NX_HalClearBSS(void)
{
    NX_MemZero(&__NX_BssStart, &__NX_BssEnd - &__NX_BssStart);
}
