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

#include <sbi.h>
#include <regs.h>

void NX_HalDirectUartPutc(char ch)
{
    sbi_console_putchar(ch);
}

int NX_HalDirectUartGetc(void)
{
    if(!(Read32(UART0_LSR) & UART0_LSR_DR))
    {
        return -1;
    }
    return (int)Read32(UART0_RBR);
}

NX_INTERFACE void NX_ConsoleSendData(char ch)
{
    NX_HalDirectUartPutc(ch);
}

void NX_HalDirectUartInit(void)
{
}

/**
 * default handler
*/
NX_WEAK_SYM void NX_HalDirectUartGetcHandler(char data)
{
    NX_ConsoleReceveData(data);
}

NX_PRIVATE NX_Error UartPollHandler(void)
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

NX_PRIVATE void NX_UartRxPollThread(void *arg)
{
    while (1)
    {
        UartPollHandler();
        NX_ThreadSleep(10);
    }
}

void NX_HalDirectUartStage2(void)
{
    NX_Thread *thread = NX_ThreadCreate("uart_rx", NX_UartRxPollThread, NX_NULL, NX_THREAD_PRIORITY_RT_MIN);
    NX_ThreadStart(thread);
}
