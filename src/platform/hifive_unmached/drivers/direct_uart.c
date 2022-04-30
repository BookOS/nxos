/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Direct uart driver 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-17      JasonHu           Init
 */

#include <xbook.h>
#include <drivers/direct_uart.h>
#include <drivers/console.h>
#include <utils/log.h>
#include <xbook/debug.h>
#include <sched/thread.h>
#include <io/delay_irq.h>

#include <sbi.h>
#include <regs.h>

void NX_HalDirectUartPutc(char ch)
{
#ifdef CONFIG_NX_UART0_FROM_SBI
    sbi_console_putchar(ch);
#else
    while(UART0_TX & UART0_TX_FULL); /* wait fifo empty */
    UART0_TX = (NX_U32)ch;
#endif
}

int NX_HalDirectUartGetc(void)
{
#ifdef CONFIG_NX_UART0_FROM_SBI
    return sbi_console_getchar();
#else
    NX_U32 data = UART0_RX & 0xff;
    return data;
#endif
}

NX_INTERFACE void NX_ConsoleSendData(char ch)
{
    if (ch == '\n') /* send '\r' ahead */
    {
        NX_HalDirectUartPutc('\r');
    }
    NX_HalDirectUartPutc(ch);
}

void NX_HalDirectUartInit(void)
{
#ifndef CONFIG_NX_UART0_FROM_SBI
    UART0_TXCTL = UART0_TXCTL_ENABLE; /* enable uart tx */
#endif
}

/**
 * default handler
*/
NX_WEAK_SYM void NX_HalDirectUartGetcHandler(char data)
{
    NX_ConsoleReceveData(data);
}

NX_PRIVATE NX_Error UartIrqHandler(NX_IRQ_Number irqno, void *arg)
{
    int data = NX_HalDirectUartGetc();
    if (data != -1)
    {
        if (data == 127) /* backspace was 127? override as '\b' */
        {
            data = '\b';
        }
        if (NX_HalDirectUartGetcHandler != NX_NULL)
        {
            NX_HalDirectUartGetcHandler(data);
        }
    }
    return data != -1 ? NX_EOK : NX_EIO;
}

void NX_HalDirectUartStage2(void)
{
#ifndef CONFIG_NX_UART0_FROM_SBI
    UART0_RXCTL = UART0_RXCTL_ENABLE; /* enable uart rx */
#endif
    UART0_IE = UART0_IE_RXWM; /* enable rx interrupt */

    /* register interrup */
    NX_ASSERT(NX_IRQ_Bind(UART0_IRQ, UartIrqHandler, NX_NULL, "Uart0", 0) == NX_EOK);
    NX_ASSERT(NX_IRQ_Unmask(UART0_IRQ) == NX_EOK);
}
