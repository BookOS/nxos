/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Memory Manage Unite 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-16      JasonHu           Init
 */

#include <mmu.h>
#include <regs.h>

#include <xbook/debug.h>
#include <io/irq.h>
#include <utils/memory.h>

#define NX_LOG_LEVEL NX_LOG_INFO
#define NX_LOG_NAME "MMU"
#include <utils/log.h>

NX_PRIVATE NX_Error UnmapOnePage(MMU *mmu, NX_Addr virAddr);
NX_INLINE NX_Error __UnmapPage(MMU *mmu, NX_Addr virAddr, NX_USize pages);

NX_PRIVATE MMU_PTE *PageWalk(MMU_PDE *pageTable, NX_Addr virAddr, NX_Bool allocPage)
{
    NX_ASSERT(pageTable);
    
    /* The page table in sv39 mode has 3 levels */
    int level;
    for (level = 2; level > 0; level--)
    {
        MMU_PTE *pte = &pageTable[GET_LEVEL_OFF(level, virAddr)];
        
        if (PTE_USED(*pte))
        {
            pageTable = (MMU_PDE *)PTE2PADDR(*pte);
            NX_ASSERT(pageTable);
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

            /* increase last level page table reference */
            void *levelPageTable = (void *)(NX_Virt2Phy((NX_Addr)pte) & NX_PAGE_ADDR_MASK);
            NX_ASSERT(pageTable);
            NX_PageIncrease(levelPageTable);
            
            *pte = PADDR2PTE(pageTable) | PTE_V;
        }
        pageTable = (MMU_PDE *)NX_Phy2Virt(pageTable);
    }
    return &pageTable[GET_LEVEL_OFF(0, virAddr)];
}

/**
 * walk addr for get pte level 0, 1, 2
 */
NX_PRIVATE NX_Error PageWalkPTE(MMU_PDE *pageTable, NX_Addr virAddr, MMU_PTE *pteArray[3])
{
    /* The page table in sv39 mode has 3 levels */
    int level;
    for (level = 2; level > 0; level--)
    {
        MMU_PTE *pte = &pageTable[GET_LEVEL_OFF(level, virAddr)];
        pteArray[level] = pte;

        if (PTE_USED(*pte))
        {
            pageTable = (MMU_PDE *)PTE2PADDR(*pte);        
            pageTable = (MMU_PDE *)NX_Phy2Virt(pageTable);
        }
        else
        {
            NX_LOG_E("map walk pte: pte on vir:%p not used!", virAddr);
            return NX_EFAULT;
        }
    }
    pteArray[0] = &pageTable[GET_LEVEL_OFF(0, virAddr)];
    return NX_EOK;
}

NX_PRIVATE NX_Bool IsVirAddrMapped(MMU *mmu, NX_Addr virAddr)
{
    MMU_PTE *pte = PageWalk(mmu->table, virAddr, NX_False);
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

NX_PRIVATE NX_Error MapOnePage(MMU *mmu, NX_Addr virAddr, NX_Addr phyAddr, NX_UArch attr)
{
    if (IsVirAddrMapped(mmu, virAddr) == NX_True)
    {
        NX_LOG_E("map page: vir:%p was mapped!", virAddr);
        return NX_EINVAL;
    }

    MMU_PDE *pageTable = (MMU_PDE *)mmu->table;

    MMU_PTE *pte = PageWalk(pageTable, virAddr, NX_True);
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
    
    *pte = PADDR2PTE(phyAddr) | attr | PTE_V;

    return NX_EOK;
}

NX_PRIVATE void *__MapPageWithPhy(MMU *mmu, NX_Addr virAddr, NX_Addr phyAddr, NX_USize size, NX_UArch attr)
{
    NX_Addr addrStart = virAddr;
    NX_Addr addrEnd = virAddr + size - 1;

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

NX_PRIVATE void *__MapPage(MMU *mmu, NX_Addr virAddr, NX_USize size, NX_UArch attr)
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
    if (phyAddr != NX_NULL)
    {
        NX_PageFree(phyAddr);
    }
    __UnmapPage(mmu, addrStart, mappedPages);
    return NX_NULL;
}

NX_PUBLIC void *MMU_MapPage(MMU *mmu, NX_Addr virAddr, NX_USize size, NX_UArch attr)
{
    NX_ASSERT(mmu);
    if (!attr)
    {
        return NX_NULL;
    }

    virAddr = virAddr & NX_PAGE_ADDR_MASK;
    size = NX_PAGE_ALIGNUP(size);
    
    NX_UArch level = NX_IRQ_SaveLevel();
    void *addr = __MapPage(mmu, virAddr, size, attr);
    NX_IRQ_RestoreLevel(level);
    return addr;
}

NX_PUBLIC void *MMU_MapPageWithPhy(MMU *mmu, NX_Addr virAddr, NX_Addr phyAddr, NX_USize size, NX_UArch attr)
{
    NX_ASSERT(mmu);
    if (!attr)
    {
        return NX_NULL;
    }

    virAddr = virAddr & NX_PAGE_ADDR_MASK;
    phyAddr = phyAddr & NX_PAGE_ADDR_MASK;
    size = NX_PAGE_ALIGNUP(size);
    
    NX_UArch level = NX_IRQ_SaveLevel();
    void *addr = __MapPageWithPhy(mmu, virAddr, phyAddr, size, attr);
    NX_IRQ_RestoreLevel(level);
    return addr;
}

NX_PRIVATE NX_Error UnmapOnePage(MMU *mmu, NX_Addr virAddr)
{
    MMU_PDE *pageTable = (MMU_PDE *)mmu->table;
    MMU_PTE *pte;
    NX_Addr phyPage;
    void *levelPageTable;

    MMU_PTE *pteArray[3] = {NX_NULL, NX_NULL, NX_NULL};
    NX_ASSERT(PageWalkPTE(pageTable, virAddr, pteArray) == NX_EOK);

    pte = pteArray[0];
    NX_ASSERT(pte != NX_NULL);
    NX_ASSERT(PTE_USED(*pte));
    NX_ASSERT(PAGE_IS_LEAF(*pte));
    phyPage = PTE2PADDR(*pte);
    NX_ASSERT(phyPage);
    NX_PageFree((void *)phyPage);   /* free leaf page*/
    *pte = 0; /* clear pte in level 0 */
    
    /* free none-leaf page */
    levelPageTable = (void *)(((NX_Addr)pte) & NX_PAGE_ADDR_MASK); /* get level page table by pte */
    if (NX_PageFree(levelPageTable) == NX_EOK)
    {
        pte = pteArray[1];
        NX_ASSERT(PTE_USED(*pte));
        NX_ASSERT(!PAGE_IS_LEAF(*pte));
        NX_ASSERT((NX_Addr)levelPageTable == PTE2PADDR(*pte));
        *pte = 0;   /* clear pte in level 1 */

        levelPageTable = (void *)(((NX_Addr)pte) & NX_PAGE_ADDR_MASK);
        if (NX_PageFree(levelPageTable) == NX_EOK)
        {
            pte = pteArray[2]; 
            NX_ASSERT(PTE_USED(*pte));
            NX_ASSERT(!PAGE_IS_LEAF(*pte));
            NX_ASSERT((NX_Addr)levelPageTable == PTE2PADDR(*pte));
            *pte = 0;   /* clear pte in level 2 */
        }
    }
    return NX_EOK;
}

NX_INLINE NX_Error __UnmapPage(MMU *mmu, NX_Addr virAddr, NX_USize pages)
{
    while (pages > 0)
    {
        UnmapOnePage(mmu, virAddr);
        virAddr += NX_PAGE_SIZE;
        pages--;
    }
    return NX_EOK;
}

NX_PUBLIC NX_Error MMU_UnmapPage(MMU *mmu, NX_Addr virAddr, NX_USize size)
{
    NX_ASSERT(mmu);

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

NX_PUBLIC void *MMU_Vir2Phy(MMU *mmu, NX_Addr virAddr)
{
    NX_ASSERT(mmu);

    NX_Addr pagePhy;
    NX_Addr pageOffset;
    
    MMU_PDE *pageTable = (MMU_PDE *)mmu->table;

    MMU_PTE *pte = PageWalk(pageTable, virAddr, NX_False);
    if (pte == NX_NULL)
    {
        NX_PANIC("vir2phy walk fault!");
    }

    if (!PTE_USED(*pte))
    {
        NX_PANIC("vir2phy pte not used!");
    }

    pagePhy = PTE2PADDR(*pte);
    pageOffset = virAddr % NX_PAGE_SIZE;
    return (void *)(pagePhy + pageOffset);
}

NX_PUBLIC void MMU_SetPageTable(NX_Addr addr)
{
    WriteCSR(satp, MAKE_SATP(addr));
    MMU_FlushTLB();
}

NX_PUBLIC NX_Addr MMU_GetPageTable(void)
{
    NX_Addr addr = ReadCSR(satp);
    return (NX_Addr)GET_ADDR_FROM_SATP(addr);
}

NX_PUBLIC void MMU_InitTable(MMU *mmu, void *pageTable, NX_Addr virStart, NX_USize size)
{
    mmu->table = pageTable;
    mmu->virStart = virStart & NX_PAGE_ADDR_MASK;
    mmu->virEnd = virStart + NX_PAGE_ALIGNUP(size);
}
