/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Init allwinner-d1 platfrom 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-17      JasonHu           Init
 */

#include <xbook.h>
#include <trap.h>
#include <clock.h>
#include <page_zone.h>
#include <platform.h>
#include <plic.h>
#include <sbi.h>
#include <regs.h>
#include <drivers/direct_uart.h>
#include <sched/smp.h>
#include <utils/log.h>

#define NX_LOG_LEVEL NX_LOG_INFO
#define NX_LOG_NAME "INIT"
#include <xbook/debug.h>

NX_INTERFACE NX_Error NX_HalPlatformInit(NX_UArch coreId)
{
    /* NOTE: init trap first before do anything */
    CPU_InitTrap(coreId);

    NX_HalDirectUartInit();

    sbi_init();
    sbi_print_version();

    NX_LOG_I("Hello, Hifve-Unmached! on core %d", coreId);
    
    PLIC_Init(NX_True);
    
    NX_HalPageZoneInit();
    
    return NX_EOK;
}

NX_INTERFACE NX_Error NX_HalPlatformStage2(void)
{
    NX_LOG_I("stage2!");

    NX_HalDirectUartStage2();

    return NX_EOK;
}
