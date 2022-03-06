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

#include <utils/memory.h>

#include <platform.h>
#include <page_zone.h>
#include <mm/page.h>
#include <mm/mmu.h>
#include <mm/page.h>
#include <arch/mmu.h>
#include <riscv.h>

#define NX_LOG_LEVEL NX_LOG_INFO
#define NX_LOG_NAME "Page"
#include <utils/log.h>

#include <xbook/debug.h>
#include <drivers/direct_uart.h>

NX_Mmu KernelMMU;

NX_PRIVATE NX_U64 KernelTable[NX_PAGE_SIZE / sizeof(NX_U64)] NX_CALIGN(NX_PAGE_SIZE);

NX_PRIVATE void NX_HalEarlyMap(NX_Mmu *mmu, NX_Addr virStart, NX_Size size)
{
    /* map kernel self */
    NX_MmuMapPageWithPhy(mmu, virStart, virStart, size,
                         NX_PAGE_ATTR_KERNEL);
    /* uart0 */
    NX_MmuMapPageWithPhy(mmu, UART0_PHY_ADDR, UART0_PHY_ADDR, NX_PAGE_SIZE,
                         NX_PAGE_ATTR_KERNEL);
    /* CLINT */
    NX_MmuMapPageWithPhy(mmu, RISCV_CLINT_PADDR, RISCV_CLINT_PADDR, 0x10000,
                         NX_PAGE_ATTR_KERNEL);
    /* PLIC */
    NX_MmuMapPageWithPhy(mmu, RISCV_PLIC_PADDR, RISCV_PLIC_PADDR, 0x4000,
                         NX_PAGE_ATTR_KERNEL);
    NX_MmuMapPageWithPhy(mmu, RISCV_PLIC_PADDR + 0x200000, RISCV_PLIC_PADDR + 0x200000, 0x4000,
                         NX_PAGE_ATTR_KERNEL);
    
    NX_LOG_I("OS map early on [%p~%p]", MEM_KERNEL_BASE, KernelMMU.earlyEnd);
}

/**
 * Init physic memory and map kernel on virtual memory.
 */
void NX_HalPageZoneInit(void)
{    
    NX_Size memSize = DRAM_SIZE_DEFAULT;
    
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
    NX_Size avaliableSize = memSize - MEM_KERNEL_SZ - MEM_SBI_SZ;
    
    NX_Size normalSize = avaliableSize / 2;
    if (normalSize > MEM_KERNEL_SPACE_SZ)
    {
        normalSize = MEM_KERNEL_SPACE_SZ;
    }
    
    /* calc user base & size */
    NX_Addr userBase = MEM_NORMAL_BASE + normalSize;
    NX_Size userSize = avaliableSize - normalSize;

    NX_LOG_I("Normal memory: %p~%p NX_Size:%d MB", MEM_NORMAL_BASE, MEM_NORMAL_BASE + normalSize, normalSize / NX_MB);
    NX_LOG_I("User memory: %p~%p NX_Size:%d MB", userBase, userBase + userSize, userSize / NX_MB);

    /* init page zone */
    NX_PageInitZone(NX_PAGE_ZONE_NORMAL, (void *)MEM_NORMAL_BASE, normalSize);
    NX_PageInitZone(NX_PAGE_ZONE_USER, (void *)userBase, userSize);

    NX_MmuInit(&KernelMMU, KernelTable, MEM_KERNEL_BASE, MEM_KERNEL_TOP, MEM_NORMAL_BASE + normalSize);

    NX_HalEarlyMap(&KernelMMU, KernelMMU.virStart, KernelMMU.earlyEnd - KernelMMU.virStart);

    NX_MmuSetPageTable((NX_UArch)KernelMMU.table);
    NX_MmuEnable();

    NX_LOG_I("MMU enabled");
    
    NX_LOG_I("Memroy init done.");
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
