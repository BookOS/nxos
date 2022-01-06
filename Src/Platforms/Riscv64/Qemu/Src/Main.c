/**
 * Copyright (c) 2018-2021, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Platfrom main 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-04     JasonHu           Init
 */

#include <XBook.h>
#include <Drivers/DirectUart.h>

#define NX_LOG_NAME "HAL_PlatformMain"
#include <Utils/Log.h>

NX_INTERFACE void HAL_PlatformMain(void)
{
    NX_LOG_I("QEMU platform main running...\n");
}

NX_PUBLIC void HAL_DirectUartGetcHandler(char data)
{
    switch (data)
    {
    case '\r':
    case '\n':
        HAL_DirectUartPutc('\n');
        break;
    case '\b':
        HAL_DirectUartPutc('\b');
        HAL_DirectUartPutc(' ');
        HAL_DirectUartPutc('\b');
        break;
    default:
        HAL_DirectUartPutc(data);
        break;
    }
}
