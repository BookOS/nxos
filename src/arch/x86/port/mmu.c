/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Memory Manage Unite 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-2-2       JasonHu           Init
 */

#include <mm/mmu.h>
#include <arch/mmu.h>
#include <mm/page.h>
#include <regs.h>
#include <xbook/debug.h>
#include <io/irq.h>
#include <utils/memory.h>

#define NX_LOG_LEVEL NX_LOG_INFO
#define NX_LOG_NAME "NX_Mmu"
#include <utils/log.h>

typedef NX_U32 MMU_PDE; /* page dir entry */
typedef NX_U32 MMU_PTE; /* page table entry */

#define __PTE_SHIFT NX_PAGE_SHIFT
#define __PTE_BITS 10
#define __PDE_SHIFT (__PTE_SHIFT + __PTE_BITS)
#define __PDE_BITS 10
#define __PAGE_PARTBIT(value, startBit, length) (((value) >> (startBit)) & ((1UL << (length)) - 1))

/* page offset */
#define GET_PF_ID(addr) ((addr) >> NX_PAGE_SHIFT)
#define GET_PF_OFFSET(addr) ((addr) & NX_PAGE_MASK)
#define GET_PDE_OFF(addr) __PAGE_PARTBIT(addr, __PDE_SHIFT, __PDE_BITS)
#define GET_PTE_OFF(addr) __PAGE_PARTBIT(addr, __PTE_SHIFT, __PTE_BITS)
#define PTE2PADDR(pte) ((pte) & NX_PAGE_ADDR_MASK)
#define PTE2ATTR(pte) ((pte) & NX_PAGE_MASK)
#define PADDR2PTE(pa) (((NX_Addr)pa) & NX_PAGE_ADDR_MASK)

#define PTE_CNT_PER_PAGE 1024

/* map kernel early need some page tables */
#define EARLY_PAGE_TABLE_ADDR     0X3F3000

#define MAKE_PTE(paddr, attr) (NX_UArch) (((NX_UArch)(paddr) & NX_PAGE_ADDR_MASK) | ((attr) & NX_PAGE_MASK))

#define PTE_USED(pte) ((pte) & PTE_P)

NX_PRIVATE NX_Error UnmapOnePage(NX_Mmu *mmu, NX_Addr virAddr);
NX_INLINE NX_Error __UnmapPage(NX_Mmu *mmu, NX_Addr virAddr, NX_USize pages);

NX_PRIVATE MMU_PTE *PageWalk(MMU_PDE *pageTable, NX_Addr virAddr, NX_Bool allocPage, NX_UArch attr)
{
    MMU_PTE *pte = &pageTable[GET_PDE_OFF(virAddr)];
    
    if (PTE_USED(*pte))
    {
        pageTable = (MMU_PDE *)PTE2PADDR(*pte);
    }
    else
    {
        if (allocPage == NX_False)
        {
            return NX_NULL;
        }
        pageTable = (MMU_PDE *)NX_PageAlloc(1);
        if (pageTable == NX_NULL)
        {
            return NX_NULL;
        }
        NX_MemZero(NX_Phy2Virt(pageTable), NX_PAGE_SIZE);

        /* increase page table reference */
        void *levelPageTable = (void *)(NX_Virt2Phy((NX_Addr)pte) & NX_PAGE_ADDR_MASK);
        NX_PageIncrease(levelPageTable);
        
        *pte = PADDR2PTE(pageTable) | attr;
    }
    pageTable = (MMU_PDE *)NX_Phy2Virt(pageTable);

    return &pageTable[GET_PTE_OFF(virAddr)];
}

/**
 * walk addr for get pte level 0, 1
 */
NX_PRIVATE NX_Error PageWalkPTE(MMU_PDE *pageTable, NX_Addr virAddr, MMU_PTE *pteArray[2])
{
    MMU_PTE *pte = (MMU_PTE *)&pageTable[GET_PDE_OFF(virAddr)];
    pteArray[1] = pte;

    if (PTE_USED(*pte))
    {
        pageTable = (MMU_PTE *)PTE2PADDR(*pte);        
        pageTable = (MMU_PTE *)NX_Phy2Virt(pageTable);
    }
    else
    {
        NX_LOG_E("map walk pte: pte on vir:%p not used!", virAddr);
        return NX_EFAULT;
    }
    pteArray[0] = &pageTable[GET_PTE_OFF(virAddr)];
    return NX_EOK;
}

NX_PRIVATE NX_Bool IsVirAddrMapped(NX_Mmu *mmu, NX_Addr virAddr)
{
    MMU_PTE *pte = PageWalk(mmu->table, virAddr, NX_False, 0);
    if (pte == NX_NULL)
    {
        return NX_False;
    }
    if (!PTE_USED(*pte))
    {
        return NX_False;
    }
    return NX_True;
}

NX_PRIVATE NX_Error MapOnePage(NX_Mmu *mmu, NX_Addr virAddr, NX_Addr phyAddr, NX_UArch attr)
{
    if (IsVirAddrMapped(mmu, virAddr) == NX_True)
    {
        NX_LOG_E("map page: vir:%p was mapped!", virAddr);
        return NX_EINVAL;
    }

    MMU_PDE *pageTable = (MMU_PDE *)mmu->table;

    MMU_PTE *pte = PageWalk(pageTable, virAddr, NX_True, attr);
    if (pte == NX_NULL)
    {
        NX_LOG_E("map page: walk page vir:%p failed!", virAddr);
        return NX_EFAULT;
    }
    if (PTE_USED(*pte))
    {
        NX_PANIC("Map one page but PTE had used!");
    }
    /* increase last level page table reference */
    void *levelPageTable = (void *)(NX_Virt2Phy((NX_Addr)pte) & NX_PAGE_ADDR_MASK);
    NX_PageIncrease(levelPageTable);
    
    *pte = PADDR2PTE(phyAddr) | attr;

    return NX_EOK;
}

NX_PRIVATE void *__MapPageWithPhy(NX_Mmu *mmu, NX_Addr virAddr, NX_Addr phyAddr, NX_USize size, NX_UArch attr)
{
    NX_Addr addrStart = phyAddr;
    NX_Addr addrEnd = phyAddr + size - 1;

    NX_ISize pages = GET_PF_ID(addrEnd) - GET_PF_ID(addrStart) + 1;
    NX_USize mappedPages = 0;

    while (pages > 0)
    {
        if (MapOnePage(mmu, virAddr, phyAddr, attr) != NX_EOK)
        {
            NX_LOG_E("map page: vir:%p phy:%p attr:%x failed!", virAddr, phyAddr, attr);
            __UnmapPage(mmu, addrStart, mappedPages);
            return NX_NULL;
        }
        virAddr += NX_PAGE_SIZE;
        phyAddr += NX_PAGE_SIZE;
        pages--;
        mappedPages++;
    }
    return (void *)addrStart;
}

NX_PRIVATE void *__MapPage(NX_Mmu *mmu, NX_Addr virAddr, NX_USize size, NX_UArch attr)
{
    NX_Addr addrStart = virAddr;
    NX_Addr addrEnd = virAddr + size - 1;

    NX_ISize pages = GET_PF_ID(addrEnd) - GET_PF_ID(addrStart) + 1;
    NX_USize mappedPages = 0;
    void *phyAddr;

    while (pages > 0)
    {
        phyAddr = NX_PageAlloc(1);
        if (phyAddr == NX_NULL)
        {
            NX_LOG_E("map page: alloc page failed!");
            goto err;
        }

        if (MapOnePage(mmu, virAddr, (NX_Addr)phyAddr, attr) != NX_EOK)
        {
            NX_LOG_E("map page: vir:%p phy:%p attr:%x failed!", virAddr, phyAddr, attr);
            goto err;
        }
        virAddr += NX_PAGE_SIZE;
        phyAddr += NX_PAGE_SIZE;
        pages--;
        mappedPages++;
    }
    return (void *)addrStart;
err:
    __UnmapPage(mmu, addrStart, mappedPages);
    return NX_NULL;
}

NX_PRIVATE void *HAL_MapPage(NX_Mmu *mmu, NX_Addr virAddr, NX_USize size, NX_UArch attr)
{
    virAddr = virAddr & NX_PAGE_ADDR_MASK;
    size = NX_PAGE_ALIGNUP(size);
    
    NX_UArch level = NX_IRQ_SaveLevel();
    void *addr = __MapPage(mmu, virAddr, size, attr);
    NX_IRQ_RestoreLevel(level);
    return addr;
}

NX_PRIVATE void *HAL_MapPageWithPhy(NX_Mmu *mmu, NX_Addr virAddr, NX_Addr phyAddr, NX_USize size, NX_UArch attr)
{
    virAddr = virAddr & NX_PAGE_ADDR_MASK;
    phyAddr = phyAddr & NX_PAGE_ADDR_MASK;
    size = NX_PAGE_ALIGNUP(size);
    
    NX_UArch level = NX_IRQ_SaveLevel();
    void *addr = __MapPageWithPhy(mmu, virAddr, phyAddr, size, attr);
    NX_IRQ_RestoreLevel(level);
    return addr;
}

NX_PRIVATE NX_Error UnmapOnePage(NX_Mmu *mmu, NX_Addr virAddr)
{
    MMU_PDE *pde, *pageTable = (MMU_PDE *)mmu->table;
    MMU_PTE *pte;
    NX_Addr phyPage;
    void *levelPageTable;

    MMU_PTE *pteArray[2] = {NX_NULL, NX_NULL};
    NX_ASSERT(PageWalkPTE(pageTable, virAddr, pteArray) == NX_EOK);

    pte = pteArray[0];
    NX_ASSERT(pte != NX_NULL);
    NX_ASSERT(PTE_USED(*pte));
    phyPage = PTE2PADDR(*pte);
    NX_ASSERT(phyPage);
    NX_PageFree((void *)phyPage);   /* free leaf page*/
    *pte = 0; /* clear pte */
    
    /* free none-leaf page */
    levelPageTable = (void *)(((NX_Addr)pte) & NX_PAGE_ADDR_MASK); /* get level page table by pte */
    if (NX_PageFree(levelPageTable) == NX_EOK)
    {
        pde = pteArray[1];
        NX_ASSERT(PTE_USED(*pde));
        NX_ASSERT((NX_Addr)levelPageTable == PTE2PADDR(*pde));
        *pde = 0;   /* clear pde */
    }
    return NX_EOK;
}

NX_INLINE NX_Error __UnmapPage(NX_Mmu *mmu, NX_Addr virAddr, NX_USize pages)
{
    while (pages > 0)
    {
        UnmapOnePage(mmu, virAddr);
        virAddr += NX_PAGE_SIZE;
        pages--;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error HAL_UnmapPage(NX_Mmu *mmu, NX_Addr virAddr, NX_USize size)
{
    virAddr = virAddr & NX_PAGE_ADDR_MASK;
    size = NX_PAGE_ALIGNUP(size);
    
    NX_Addr addrStart = virAddr;
    NX_Addr addrEnd = virAddr + size - 1;
    NX_USize pages = GET_PF_ID(addrEnd) - GET_PF_ID(addrStart) + 1;

    NX_UArch level = NX_IRQ_SaveLevel();
    NX_Error err = __UnmapPage(mmu, virAddr, pages);
    NX_IRQ_RestoreLevel(level);
    return err;
}

NX_PRIVATE void *HAL_Vir2Phy(NX_Mmu *mmu, NX_Addr virAddr)
{
    NX_Addr pagePhy;
    NX_Addr pageOffset;
    
    MMU_PDE *pageTable = (MMU_PDE *)mmu->table;

    MMU_PTE *pte = PageWalk(pageTable, virAddr, NX_False, 0);
    if (pte == NX_NULL)
    {
        NX_PANIC("vir2phy walk fault!");
    }

    if (!PTE_USED(*pte))
    {
        NX_PANIC("vir2phy pte not used!");
    }

    pagePhy = PTE2PADDR(*pte);

    //NX_LOG_E("ATTR:%x", PTE2ATTR(*pte));
    
    pageOffset = virAddr % NX_PAGE_SIZE;
    return (void *)(pagePhy + pageOffset);
}

NX_PRIVATE void HAL_EarlyMap(NX_Mmu *mmu, NX_Addr virStart, NX_USize size)
{
    virStart = virStart & NX_PAGE_ADDR_MASK;
    NX_Addr phyStart = virStart;
    NX_Addr virEnd = virStart + NX_PAGE_ALIGNUP(size);

    NX_LOG_I("OS map early on [%p~%p]", virStart, virEnd);
    
    MMU_PDE *pdt = (MMU_PDE *)mmu->table;
    
    NX_USize pdeCnt = (virEnd - virStart) / (PTE_CNT_PER_PAGE * NX_PAGE_SIZE);
    NX_USize pteCnt = ((virEnd - virStart) / NX_PAGE_SIZE) % PTE_CNT_PER_PAGE;
    NX_Addr *pte = (NX_UArch *) EARLY_PAGE_TABLE_ADDR;
    NX_U32 pdeIdx = GET_PDE_OFF(virStart);
    int i, j;
    for (i = 0; i < pdeCnt; i++)
    {
        /* fill pde */
        pdt[pdeIdx + i] = MAKE_PTE(pte, ARCH_PAGE_ATTR_KERNEL);
        for (j = 0; j < PTE_CNT_PER_PAGE; j++)
        {
            /* fill each pte */
            pte[j] = MAKE_PTE(phyStart, ARCH_PAGE_ATTR_KERNEL);
            phyStart += NX_PAGE_SIZE;
        }
        pte += PTE_CNT_PER_PAGE;
    }
    if (pteCnt > 0)
    {
        /* fill left pte */
        pdt[pdeIdx + i] = MAKE_PTE(pte, ARCH_PAGE_ATTR_KERNEL);
        for (j = 0; j < pteCnt; j++)
        {
            pte[j] = MAKE_PTE(phyStart, ARCH_PAGE_ATTR_KERNEL);
            phyStart += NX_PAGE_SIZE;
        }
    }
}

NX_PRIVATE void HAL_SetPageTable(NX_Addr addr)
{
    /* set new pgdir will flush tlb */
    CPU_WriteCR3(addr);
}

NX_PRIVATE NX_Addr HAL_GetPageTable(void)
{
    return CPU_ReadCR3();
}

NX_PRIVATE void HAL_Enable(void)
{
    CPU_WriteCR0(CPU_ReadCR0() | CR0_PG);
}

NX_INTERFACE struct NX_MmuOps NX_MmuOpsInterface = 
{
    .setPageTable   = HAL_SetPageTable,
    .getPageTable   = HAL_GetPageTable,
    .enable         = HAL_Enable,
    .earlyMap       = HAL_EarlyMap,
    .mapPage        = HAL_MapPage,
    .mapPageWithPhy = HAL_MapPageWithPhy,
    .unmapPage      = HAL_UnmapPage,
    .vir2Phy        = HAL_Vir2Phy,
};
