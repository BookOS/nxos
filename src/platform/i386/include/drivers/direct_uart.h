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

void HAL_DirectUartInit(void);
void HAL_DirectUartPutc(char ch);
void HAL_DirectUartStage2(void);

#endif /* __DIRECT_UART_HEADER__ */
