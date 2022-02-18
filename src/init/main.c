/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Init OS 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-3      JasonHu           Init
 */

#define NX_LOG_NAME "OS Main"
#include <utils/log.h>
#include <xbook/debug.h>

#include <mods/test/utest.h>
#include <sched/thread.h>
#include <sched/sched.h>
#include <sched/smp.h>
#include <mm/heap_cache.h>
#include <mm/page_cache.h>
#include <io/irq.h>
#include <mods/time/timer.h>

/* Platform init */
NX_INTERFACE NX_Error HAL_PlatformInit(NX_UArch coreId);

/**
 * stage2 means you can do:
 * 1. use NX_MemAlloc/NX_MemFree
 * 2. use Bind IRQ
 * 3. use NX_Thread
 * 4. use Timer
 */
NX_INTERFACE NX_WEAK_SYM NX_Error HAL_PlatformStage2(void)
{
    return NX_EOK;
}

NX_IMPORT NX_Atomic NX_ActivedCoreCount;

NX_PUBLIC int NX_Main(NX_UArch coreId)
{
    if (NX_AtomicGet(&NX_ActivedCoreCount) == 0)
    {
        NX_AtomicInc(&NX_ActivedCoreCount);
        /* init multi core before enter platform */
        NX_SMP_Preload(coreId);
        
        /* platfrom init */
        if (HAL_PlatformInit(coreId) != NX_EOK)
        {
            NX_PANIC("Platfrom init failed!");
        }
        NX_LOG_I("Hello, NXOS on %s!", CONFIG_NX_PLATFROM_NAME);

        /* init irq */
        NX_IRQ_Init();

        /* init page cache */
        NX_PageCacheInit();
        
        /* init heap cache for NX_MemAlloc & NX_MemFree */
        NX_HeapCacheInit();
        
        /* init timer */
        NX_TimersInit();

        /* init multi core */
        NX_SMP_Init(coreId);

        /* init thread */
        NX_ThreadsInit();
        
        if (NX_ClockInit() != NX_EOK)
        {
            NX_PANIC("Clock init failed!");
        }
        
        /* init auto calls */
        NX_CallsInit();
        
        /* platform stage2 call */
        if (HAL_PlatformStage2() != NX_EOK)
        {
            NX_PANIC("Platform stage2 failed!");
        }
        
        NX_SMP_Main(coreId);
    }
    else
    {
        NX_AtomicInc(&NX_ActivedCoreCount);
        NX_SMP_Stage2(coreId);
    }
    /* start sched */
    NX_SchedToFirstThread();
    /* should never be here */
    NX_PANIC("should never be here!");
    return 0;
}
