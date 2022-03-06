/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Platfrom main 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-18     JasonHu           Init
 */

#include <xbook.h>
#include <drivers/direct_uart.h>

#define NX_LOG_NAME "Hal Main"
#include <utils/log.h>

NX_INTERFACE void NX_HalPlatformMain(void)
{
    NX_LOG_I("PC32 platform main running...\n");

}

void NX_HalDirectUartGetcHandler(char data)
{
    switch (data)
    {
    case '\r':
    case '\n':
        NX_HalDirectUartPutc('\n');
        break;
    case '\b':
        NX_HalDirectUartPutc('\b');
        NX_HalDirectUartPutc(' ');
        NX_HalDirectUartPutc('\b');
        break;
    default:
        NX_HalDirectUartPutc(data);
        break;
    }
}
