/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Direct uart
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-4      JasonHu           Init
 */

#ifndef __DIRECT_UART_HEADER__
#define __DIRECT_UART_HEADER__

#include <nxos.h>

#define UART0_PHY_ADDR 0x38000000L
#define UART0_IRQ 33

/* direct means not use driver framework */

void NX_HalDirectUartInit(void);
void NX_HalDirectUartStage2(void);

void NX_HalDirectUartPutc(char ch);
int NX_HalDirectUartGetc(void);

#endif /* __DIRECT_UART_HEADER__ */
