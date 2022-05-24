/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Init OS 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-3      JasonHu           Init
 */

#define NX_LOG_NAME "OS Main"
#include <base/log.h>
#include <base/debug.h>

#include <test/utest.h>
#include <base/thread.h>
#include <base/sched.h>
#include <base/smp.h>
#include <base/heap_cache.h>
#include <base/page_cache.h>
#include <base/irq.h>
#include <base/timer.h>

/**
 * see http://asciiarts.net
 * Text: "NXOS", Font: standard.flf
 */
NX_PRIVATE char *logString = \
"    _   ___  ______  _____\n"\
"   / | / / |/ / __ \\/ ___/\n"\
"  /  |/ /|   / / / /\\__ \\ \n"\
" / /|  //   / /_/ /___/ / \n"\
"/_/ |_//_/|_\\____//____/  \n";

NX_IMPORT NX_Atomic gActivedCoreCount;

/* Platform init */
NX_INTERFACE NX_Error NX_HalPlatformInit(NX_UArch coreId);

/**
 * stage2 means you can do:
 * 1. use NX_MemAlloc/NX_MemFree
 * 2. use Bind IRQ
 * 3. use NX_Thread
 * 4. use Timer
 */
NX_INTERFACE NX_WEAK_SYM NX_Error NX_HalPlatformStage2(void)
{
    return NX_EOK;
}

NX_PRIVATE void ShowLogVersion(void)
{
    NX_Printf("%s\n", logString);
    NX_Printf("Kernel    : %s\n", NX_SYSTEM_NAME);
    NX_Printf("Version   : %d.%d.%d\n", NX_VERSION_MAJOR, NX_VERSION_MINOR, NX_VERSION_REVISE);
    NX_Printf("Build     : %s\n", __DATE__);
    NX_Printf("Platform  : %s\n", CONFIG_NX_PLATFORM_NAME);
    NX_Printf("Copyright (c) 2018-2022, NXOS Development Team\n");
}

int NX_Main(NX_UArch coreId)
{
    if (NX_AtomicGet(&gActivedCoreCount) == 0)
    {
        NX_AtomicInc(&gActivedCoreCount);
        /* init multi core before enter platform */
        NX_SMP_Preload(coreId);
        
        /* platfrom init */
        if (NX_HalPlatformInit(coreId) != NX_EOK)
        {
            NX_PANIC("Platfrom init failed!");
        }

        ShowLogVersion();

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
        if (NX_HalPlatformStage2() != NX_EOK)
        {
            NX_PANIC("Platform stage2 failed!");
        }
        
        NX_SMP_Main(coreId);
    }
    else
    {
        NX_AtomicInc(&gActivedCoreCount);
        NX_SMP_Stage2(coreId);
    }
    /* start sched */
    NX_SchedToFirstThread();
    /* should never be here */
    NX_PANIC("should never be here!");
    return 0;
}
