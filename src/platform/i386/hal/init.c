/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Init PC32 platfrom 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-9-17      JasonHu           Init
 */

#include <xbook.h>
#include <segment.h>
#include <gate.h>
#include <interrupt.h>
#include <tss.h>
#include <page_zone.h>
#include <platform.h>

#define NX_LOG_LEVEL NX_LOG_INFO
#define NX_LOG_NAME "INIT"
#include <utils/log.h>

#include <drivers/direct_uart.h>

NX_INTERFACE NX_Error NX_HalPlatformInit(NX_UArch coreId)
{
    NX_HalClearBSS();
    
    NX_HalDirectUartInit();
    
    NX_Printf("hello, world!\na=%x b=%d c=%c e=%s\n", 0x1234abcd, 123456789, 'A', "hello");
    
    NX_LOG_I("Hello, PC32!");

    CPU_InitGate();
    CPU_InitSegment();
    CPU_InitTSS();
    CPU_InitInterrupt();
    
    NX_HalPageZoneInit();

    return NX_EOK;
}

NX_INTERFACE NX_Error NX_HalPlatformStage2(void)
{
    NX_HalDirectUartStage2();
    
    return NX_EOK;
}
