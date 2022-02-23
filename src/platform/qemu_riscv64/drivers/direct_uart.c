/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Direct uart driver 
 * low-level driver routines for 16550a UART.
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-1      JasonHu           Init
 */

#include <xbook.h>
#include <io/delay_irq.h>
#include <drivers/direct_uart.h>
#include <utils/log.h>
#include <xbook/debug.h>

#ifdef CONFIG_NX_UART0_FROM_SBI
#include <sbi.h>
#endif

#include <regs.h>

// the UART control registers.
// some have different meanings for
// read vs write.
// see http://byterunner.com/16550.html
#define RHR 0                  // receive holding register (for input bytes)
#define THR 0                  // transmit holding register (for output bytes)
#define IER 1                  // interrupt enable register
#define IER_RX_ENABLE (1 << 0) // receiver ready interrupt.
#define IER_TX_ENABLE (1 << 1) // transmitter empty interrupt.
#define FCR 2                  // FIFO control register
#define FCR_FIFO_ENABLE (1 << 0)
#define FCR_FIFO_CLEAR (3 << 1) // clear the content of the two FIFOs
#define ISR 2                   // interrupt status register
#define LCR 3                   // line control register
#define LCR_EIGHT_BITS (3 << 0)
#define LCR_BAUD_LATCH (1 << 7) // special mode to set baud rate
#define LSR 5                   // line status register
#define LSR_RX_READY (1 << 0)   // input is waiting to be read from RHR
#define LSR_TX_IDLE (1 << 5)    // THR can accept another character to send

void HAL_DirectUartPutc(char ch)
{
#ifdef CONFIG_NX_UART0_FROM_SBI
    sbi_console_putchar(ch);
#else
    if ((Read8(UART0_PHY_ADDR + LSR) & LSR_TX_IDLE) == 0)
    {
        // the UART transmit holding register is full,
        return;
    }
    Write8(UART0_PHY_ADDR + THR, ch);
#endif
}

int HAL_DirectUartGetc(void)
{
#ifdef CONFIG_NX_UART0_FROM_SBI
    return sbi_console_getchar();
#else
    if (Read8(UART0_PHY_ADDR + LSR) & 0x01)
    {
        // input data is ready.
        return Read8(UART0_PHY_ADDR + RHR);
    }
    else
    {
        return -1;
    }
#endif
}

NX_INTERFACE void HAL_ConsoleOutChar(char ch)
{
    HAL_DirectUartPutc(ch);
}

void HAL_DirectUartInit(void)
{
    // disable interrupts.
    Write8(UART0_PHY_ADDR + IER, 0x00);
    // special mode to set baud rate.
    Write8(UART0_PHY_ADDR + LCR, LCR_BAUD_LATCH);
    // LSB for baud rate of 115.2K.
    Write8(UART0_PHY_ADDR + 0, 0x01);
    // MSB for baud rate of 115.2K.
    Write8(UART0_PHY_ADDR + 1, 0x00);
    // leave set-baud mode,
    // and set word length to 8 bits, no parity.
    Write8(UART0_PHY_ADDR + LCR, LCR_EIGHT_BITS);
    // reset and enable FIFOs.
    Write8(UART0_PHY_ADDR + FCR, FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);
}

/**
 * default handler
*/
NX_WEAK_SYM void HAL_DirectUartGetcHandler(char data)
{
    NX_LOG_I("Deafult uart handler:%x/%c\n", data, data);
}

NX_PRIVATE NX_Error UartIrqHandler(NX_IRQ_Number irqno, void *arg)
{
    int data = HAL_DirectUartGetc();
    if (data != -1)
    {
        if (HAL_DirectUartGetcHandler != NX_NULL)
        {
            HAL_DirectUartGetcHandler(data);
        }
    }
    return data != -1 ? NX_EOK : NX_EIO;
}

void HAL_DirectUartStage2(void)
{
    /* enable receive interrupts. */
    Write8(UART0_PHY_ADDR + IER, IER_RX_ENABLE);

    NX_ASSERT(NX_IRQ_Bind(UART0_IRQ, UartIrqHandler, NX_NULL, "Uart", 0) == NX_EOK);
    NX_ASSERT(NX_IRQ_Unmask(UART0_IRQ) == NX_EOK);
}
