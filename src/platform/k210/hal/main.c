/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Platfrom main 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-04     JasonHu           Init
 */

#include <xbook.h>
#include <drivers/direct_uart.h>

#define NX_LOG_NAME "Hal Main"
#include <utils/log.h>

#ifdef CONFIG_NX_ENABLE_PLATFORM_MAIN
NX_INTERFACE void NX_HalPlatformMain(void)
{
    NX_LOG_I("k210 platform main running...\n");
}
#endif /* CONFIG_NX_ENABLE_PLATFORM_MAIN */

void NX_HalDirectUartGetcHandler(char data)
{
    switch (data)
    {
    case '\r':
        break; /* on k210, "enter" will input "\n" and "\r", ignore "\r" here. */
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
