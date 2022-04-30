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

#include <xbook.h>

#define UART0_PHY_ADDR 0x10010000UL

#define UART0_IRQ 39

#define UART0_TX                (*(NX_U32 volatile *)(UART0_PHY_ADDR + 0x00))
#define UART0_TX_FULL           (1UL << 31) /* transmit FIFO full */
#define UART0_TXCTL             (*(NX_U32 volatile *)(UART0_PHY_ADDR + 0x08))
#define UART0_TXCTL_ENABLE      0x01    /* transmit enable */
#define UART0_RX                (*(NX_U32 volatile *)(UART0_PHY_ADDR + 0x04))
#define UART0_RXCTL             (*(NX_U32 volatile *)(UART0_PHY_ADDR + 0x0c))
#define UART0_RXCTL_ENABLE      0x01    /* receive enable */
#define UART0_IE                (*(NX_U32 volatile *)(UART0_PHY_ADDR + 0x10))
#define UART0_IE_RXWM           0X02 /* receive interrupt wartermark interrupt enable */

void NX_HalDirectUartInit(void);
void NX_HalDirectUartStage2(void);

void NX_HalDirectUartPutc(char ch);
int NX_HalDirectUartGetc(void);

#endif /* __DIRECT_UART_HEADER__ */
