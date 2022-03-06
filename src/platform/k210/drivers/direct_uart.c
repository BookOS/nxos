/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Direct uart driver from rustsbi
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-26      JasonHu           Init
 */

#include <xbook.h>
#include <io/delay_irq.h>
#include <drivers/direct_uart.h>
#include <utils/log.h>
#include <xbook/debug.h>
#include <sbi.h>

#include <regs.h>

void NX_HalDirectUartPutc(char ch)
{
    sbi_console_putchar(ch);
}

int NX_HalDirectUartGetc(void)
{
    return sbi_console_getchar();
}

NX_INTERFACE void NX_HalConsoleOutChar(char ch)
{
    NX_HalDirectUartPutc(ch);
}

void NX_HalDirectUartInit(void)
{
    /* k210 do nothing */
}

/**
 * default handler
*/
NX_WEAK_SYM void NX_HalDirectUartGetcHandler(char data)
{
    NX_LOG_I("Deafult uart handler:%x/%c\n", data, data);
}

NX_PRIVATE NX_Error UartIrqHandler(NX_IRQ_Number irqno, void *arg)
{
    int data = NX_HalDirectUartGetc();
    if (data != -1)
    {
        if (NX_HalDirectUartGetcHandler != NX_NULL)
        {
            NX_HalDirectUartGetcHandler(data);
        }
    }
    return data != -1 ? NX_EOK : NX_EIO;
}

void NX_HalDirectUartStage2(void)
{
    NX_ASSERT(NX_IRQ_Bind(UART0_IRQ, UartIrqHandler, NX_NULL, "Uart0", 0) == NX_EOK);
    NX_ASSERT(NX_IRQ_Unmask(UART0_IRQ) == NX_EOK);
}
