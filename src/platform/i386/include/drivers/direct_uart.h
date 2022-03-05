/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Direct uart
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-3      JasonHu           Init
 */

#ifndef __DIRECT_UART_HEADER__
#define __DIRECT_UART_HEADER__

#include <xbook.h>

/* direct means not use driver framework */

void NX_HalDirectUartInit(void);
void NX_HalDirectUartPutc(char ch);
void NX_HalDirectUartStage2(void);

#endif /* __DIRECT_UART_HEADER__ */
