/**
 * Copyright (c) 2018-2022, BookOS Development Team
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
#define NX_LOG_NAME "Multi Core"
#include <utils/log.h>

#include <sbi.h>
#include <trap.h>
#include <clock.h>
#include <plic.h>
#include <regs.h>

NX_IMPORT NX_Addr TrapEntry0;
NX_IMPORT NX_Addr TrapEntry1;
NX_IMPORT NX_Addr TrapEntry2;
NX_IMPORT NX_Addr TrapEntry3;

/**
 * Within SBI, we can't read mhartid, so I try to use `trap entry` to see who am I.
 */
NX_PRIVATE NX_UArch NX_HalCoreGetIndex(void)
{
    NX_Addr trapEntry = ReadCSR(stvec);

    if (trapEntry == (NX_Addr)&TrapEntry0)
    {
        return 0;
    }
    else if (trapEntry == (NX_Addr)&TrapEntry1)
    {
        return 1;
    }
    else if (trapEntry == (NX_Addr)&TrapEntry2)
    {
        return 2;
    }
    else if (trapEntry == (NX_Addr)&TrapEntry3)
    {
        return 3;
    }
    /* should never be here */
    while (1);
}

NX_PRIVATE NX_Error NX_HalCoreBootApp(NX_UArch bootCoreId)
{
#ifdef CONFIG_NX_PLATFROM_K210
    return NX_ENORES;   /* not support smp on k210 */
#else
    NX_LOG_I("boot core is:%d", bootCoreId);
    NX_UArch coreId;
    for (coreId = 0; coreId < NX_MULTI_CORES_NR; coreId++)
    {
        NX_LOG_I("core#%d state:%d", coreId, sbi_hsm_hart_status(coreId));
        if (bootCoreId == coreId) /* skip boot core */
        {
            NX_LOG_I("core#%d is boot core, skip it", coreId);
            continue;
        }
        NX_LOG_I("wakeup app core:%d", coreId);
        sbi_hsm_hart_start(coreId, MEM_KERNEL_BASE, 0);    
        NX_MemoryBarrier();
        NX_LOG_I("core#%d state:%d after wakeup", coreId, sbi_hsm_hart_status(coreId));
    }
    return NX_EOK;
#endif
}

NX_PRIVATE NX_Error NX_HalCoreEnterApp(NX_UArch appCoreId)
{
    /* NOTE: init trap first before do anything */
    CPU_InitTrap(appCoreId);
    NX_LOG_I("core#%d enter application!", appCoreId);
    PLIC_Init(NX_False);

    return NX_EOK;
}

NX_INTERFACE struct NX_SMP_Ops NX_SMP_OpsInterface = 
{
    .getIdx = NX_HalCoreGetIndex,
    .bootApp = NX_HalCoreBootApp,
    .enterApp = NX_HalCoreEnterApp,
};
