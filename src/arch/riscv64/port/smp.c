/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: HAL Multi Core support
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-9      JasonHu           Init
 */

#include <xbook.h>
#include <sched/smp.h>
#include <mm/barrier.h>
#include <platform.h>
#define NX_LOG_NAME "smp-riscv64"
#include <utils/log.h>

#include <sbi.h>
#include <trap.h>
#include <clock.h>
#include <plic.h>
#include <regs.h>
#include <mm/mmu.h>

NX_IMPORT NX_Addr gTrapEntry0;
NX_IMPORT NX_Addr gTrapEntry1;
NX_IMPORT NX_Addr gTrapEntry2;
NX_IMPORT NX_Addr gTrapEntry3;
NX_IMPORT NX_Addr gTrapEntry4;

/**
 * Within SBI, we can't read mhartid, so I try to use `trap entry` to see who am I.
 */
NX_PRIVATE NX_UArch NX_HalCoreGetIndex(void)
{
    NX_Addr trapEntry = ReadCSR(stvec);

    if (trapEntry == (NX_Addr)&gTrapEntry0)
    {
        return 0;
    }
    else if (trapEntry == (NX_Addr)&gTrapEntry1)
    {
        return 1;
    }
    else if (trapEntry == (NX_Addr)&gTrapEntry2)
    {
        return 2;
    }
    else if (trapEntry == (NX_Addr)&gTrapEntry3)
    {
        return 3;
    }
    else if (trapEntry == (NX_Addr)&gTrapEntry4)
    {
        return 4;
    }
    /* should never be here */
    while (1);
}

NX_PRIVATE NX_Error NX_HalCoreBootApp(NX_UArch bootCoreId)
{

    NX_LOG_I("boot core is:%d", bootCoreId);
    NX_UArch coreId;
    for (coreId = NX_VALID_HARTID_OFFSET; coreId < NX_MULTI_CORES_NR; coreId++)
    {
#ifndef CONFIG_NX_PLATFORM_K210
        NX_LOG_I("core#%d state:%d", coreId, sbi_hsm_hart_status(coreId));
#endif
        if (bootCoreId == coreId) /* skip boot core */
        {
            NX_LOG_I("core#%d is boot core, skip it", coreId);
            continue;
        }
        NX_LOG_I("wakeup app core:%d", coreId);
#ifdef CONFIG_NX_PLATFORM_K210
        NX_UArch mask = 1 << coreId;
        sbi_send_ipi(&mask);
#else
        sbi_hsm_hart_start(coreId, MEM_KERNEL_BASE, 0);    
#endif
        NX_MemoryBarrier();
#ifndef CONFIG_NX_PLATFORM_K210
        NX_LOG_I("core#%d state:%d after wakeup", coreId, sbi_hsm_hart_status(coreId));
#endif
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error NX_HalCoreEnterApp(NX_UArch appCoreId)
{
    /* NOTE: init trap first before do anything */
    CPU_InitTrap(appCoreId);
    NX_LOG_I("core#%d enter application!", appCoreId);
    PLIC_Init(NX_False);

    /* enable mmu on this core */
    NX_MmuSetPageTable((NX_Addr)NX_HalGetKernelPageTable());
    NX_MmuEnable();

    return NX_EOK;
}

NX_INTERFACE struct NX_SMP_Ops NX_SMP_OpsInterface = 
{
    .getIdx = NX_HalCoreGetIndex,
    .bootApp = NX_HalCoreBootApp,
    .enterApp = NX_HalCoreEnterApp,
};
