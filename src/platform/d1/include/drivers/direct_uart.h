/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Direct uart
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-17      JasonHu           Init
 */

#ifndef __DIRECT_UART_HEADER__
#define __DIRECT_UART_HEADER__

#include <nxos.h>

#define UART0_PHY_ADDR 0x02500000UL

#define UART0_RBR       (UART0_PHY_ADDR + 0x00)    /* receive buffer register */
#define UART0_LSR       (UART0_PHY_ADDR + 0x14)    /* line status register */
#define UART0_LSR_DR    0x01    /* LSR data ready */

/* direct means not use driver framework */

void NX_HalDirectUartInit(void);
void NX_HalDirectUartStage2(void);

void NX_HalDirectUartPutc(char ch);
int NX_HalDirectUartGetc(void);

#endif /* __DIRECT_UART_HEADER__ */
