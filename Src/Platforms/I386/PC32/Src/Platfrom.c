/**
 * Copyright (c) 2018-2021, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Init PC32 platfrom 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-9-17      JasonHu           Init
 */

#include <XBook.h>
#include <Assert.h>
#include <HAL.h>
#include <Mods/Console/Console.h>
#include <Utils/List.h>
#include <Utils/Memory.h>

#include <Drivers/DirectUart.h>
#include <Segment.h>
#include <Gate.h>
#include <Interrupt.h>
#include <TSS.h>
#include <Clock.h>
#include <Context.h>
#include <MM/buddy.h>
#include <MM/Page.h>
#include <MM/PageHeap.h>
#include <Page.h>
#include <Platfrom.h>
#include <MMU.h>

IMPORT void PlatfromTest(void);

IMPORT Addr __bssStart;
IMPORT Addr __bssEnd;

PRIVATE void PageInit(void);
PRIVATE void ClearBSS(void);

PUBLIC MMU kernelMMU;

PRIVATE NOOPT U32 kernelTable[PAGE_SIZE / sizeof(U32)] CALIGN(PAGE_SIZE);

INTERFACE OS_Error PlatformInit(void)
{
    ClearBSS();
    
    HAL_DirectUartInit();
    
    Cout("Hello, PC32!" Endln);

    CPU_InitGate();
    CPU_InitSegment();
    CPU_InitTSS();
    CPU_InitInterrupt();
    
    if (HAL_InitClock() != OS_EOK)
    {
        Cout("Init clock failed!" Endln);
        return OS_ERROR;
    }
    PageInit();
    
    PlatfromTest();
    
    SPIN("test");
    // HAL_InterruptEnable();

    return OS_EOK;
}

PRIVATE void ClearBSS(void)
{
    Zero(&__bssStart, &__bssEnd - &__bssStart);
}

#define GRUB2_READ_MEMORY_BYTES_ADDR (0x000001000)

PRIVATE void PageInit(void)
{    
    U32 memSize = *(U32 *)GRUB2_READ_MEMORY_BYTES_ADDR;
    
    Cout("Memory Size: " $x(memSize) " Bytes " $d(memSize / SZ_MB) " MB" Endln);

    if (memSize == 0)
    {
        PANIC("Get Memory Size Failed!");
    }
    if (memSize < MEM_MIN_SIZE)
    {
        Cout("Must has " $d(MEM_MIN_SIZE / SZ_MB) " MB memory!");
        PANIC("Memory too small");
    }
    
    /* calc normal base & size */
    U32 normalSize = memSize - MEM_DMA_SIZE - MEM_KERNEL_SZ;
    normalSize /= 2;
    if (normalSize + MEM_NORMAL_BASE > MEM_KERNEL_TOP)
    {
        normalSize = MEM_KERNEL_TOP - MEM_NORMAL_BASE;
    }
    
    /* calc user base & size */
    Addr userBase = MEM_NORMAL_BASE + normalSize;
    Size userSize = memSize - userBase;

    Cout("DMA memory base: " $x(MEM_DMA_BASE) " Size:" $d(MEM_DMA_SIZE / SZ_MB) " MB" Endln);
    Cout("Normal memory base: " $x(MEM_NORMAL_BASE) " Size:" $d(normalSize / SZ_MB) " MB" Endln);
    Cout("User memory base: " $x(userBase) " Size:" $d(userSize / SZ_MB) " MB" Endln);

    /* init page zone */
    PageInitZone(PZ_DMA, (void *)MEM_DMA_BASE, MEM_DMA_SIZE);
    PageInitZone(PZ_NORMAL, (void *)MEM_NORMAL_BASE, normalSize);
    PageInitZone(PZ_USER, (void *)userBase, userSize);

    kernelMMU.virStart = 0;
    kernelMMU.earlyEnd = userBase;
    kernelMMU.virEnd = MEM_KERNEL_TOP;
    kernelMMU.table = kernelTable;

    MMU_EarlyMap(&kernelMMU, kernelMMU.virStart, kernelMMU.earlyEnd);

    MMU_SetPageTable((Uint)kernelMMU.table);
    MMU_Enable();

    Cout("MMU enabled\n");

    /* init memory allocator */
    PageHeapInit();

}