/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 * SPDX-License-Identifier: Apache-2.0
 *
 * Contains: Direct uart driver
 * 
 * Change Logs:
 * Date           Author        Notes
 * 2021-10-1     JasonHu       first version
 */

#include <xbook.h>
#include <io/delay_irq.h>
#include <drivers/direct_uart.h>
#include <utils/log.h>
#include <xbook/debug.h>

#include <io.h>
#include <interrupt.h>

#define UART_SEND_TIMEOUT       1

/* I/O port base addr */
#define UART0_BASE              0X3F8
#define BAUD_VALUE_MAX          115200
#define BAUD_VALUE_DEFAULT      115200
#define BAUD_DIVISOR_DEFAULT    (BAUD_VALUE_MAX / BAUD_VALUE_DEFAULT)

enum UartFifoControl
{
    FIFO_ENABLE = 1,                             /* Enable FIFOs */
    FIFO_CLEAR_RECEIVE   = (1 << 1),             /* Clear Receive FIFO */
    FIFO_CLEAR_TRANSMIT  = (1 << 2),             /* Clear Transmit FIFO */
    FIFO_DMA_MODE_SELECT = (1 << 3),             /* DMA Mode Select */
    FIFO_RESERVED        = (1 << 4),             /* Reserved */
    FIFO_ENABLE_64       = (1 << 5),             /* Enable 64 Byte FIFO(16750) */
    /* Interrupt Trigger Level/Trigger Level  */
    FIFO_TRIGGER_1       = (0 << 6),             /* 1 Byte */
    FIFO_TRIGGER_4       = (1 << 6),             /* 4 Byte */
    FIFO_TRIGGER_8       = (1 << 7),             /* 8 Byte */
    FIFO_TRIGGER_14      = (1 << 6) | (1 << 7),  /* 14 Byte */
};

enum UartLineControl
{
    /* Word Length */
    LINE_WORD_LENGTH_5   = 0,                    /* 5 Bits */
    LINE_WORD_LENGTH_6   = 1,                    /* 6 Bits */
    LINE_WORD_LENGTH_7   = (1 << 1),             /* 7 Bits */
    LINE_WORD_LENGTH_8   = ((1 << 1) | 1),       /* 8 Bits */
    LINE_STOP_BIT_1      = (0 << 2),             /* One Stop Bit */
    LINE_STOP_BIT_2      = (1 << 2),             /* 1.5 Stop Bits or 2 Stop Bits */
    /* Parity Select */
    LINE_PARITY_NO       = (0 << 3),             /* No Parity */
    LINE_PARITY_ODD      = (1 << 3),             /* Odd Parity */
    LINE_PARITY_EVEN     = (1 << 3) | (1 << 4),  /* Even Parity */
    LINE_PARITY_MARK     = (1 << 3) | (1 << 5),  /* Mark */
    LINE_PARITY_SPACE    = (1 << 3) | (1 << 4) | (1 << 5), /* Space */
    LINE_BREAK_ENABLE    = (1 << 6),             /* Set Break Enable */
    LINE_DLAB            = (1 << 7),             /* Divisor Latch Access Bit */
};

enum UartInterruptEnable
{
    INTR_RECV_DATA_AVALIABLE = 1,        /* Enable Received Data Available Interrupt */
    INTR_TRANSMIT_HOLDING    = (1 << 1), /* Enable Transmitter Holding Register Empty Interrupt */
    INTR_STATUS_CHANGED      = (1 << 2), /* Enable Receiver Line Status Interrupt */
    INTR_MODEM_STATUS        = (1 << 3), /* Enable Modem Status Interrupt */
    INTR_SLEEP_MODE          = (1 << 4), /* Enable Sleep Mode(16750) */
    INTR_LOW_POWER_MODE      = (1 << 5), /* Enable Low Power Mode(16750) */
    INTR_RESERVED1           = (1 << 6), /* Reserved */
    INTR_RESERVED2           = (1 << 7), /* Reserved */
};

enum UartLineStatus
{
    LINE_STATUS_DATA_READY                  = 1,        /* Data Ready */
    LINE_STATUS_OVERRUN_ERROR               = (1 << 1), /* Overrun Error */
    LINE_STATUS_PARITY_ERROR                = (1 << 2), /* Parity Error */
    LINE_STATUS_FRAMING_ERROR               = (1 << 3), /* Framing Error */
    LINE_STATUS_BREAK_INTERRUPT             = (1 << 4), /* Break Interrupt */
    LINE_STATUS_EMPTY_TRANSMITTER_HOLDING   = (1 << 5), /* Empty Transmitter Holding Register */
    LINE_STATUS_EMPTY_DATA_HOLDING          = (1 << 6), /* Empty Data Holding Registers */
    LINE_STATUS_ERROR_RECEIVE_FIFO          = (1 << 7), /* Error in Received FIFO */
};

enum UartIntrIndenty
{
    INTR_STATUS_PENDING_FLAG        = 1,        /* Interrupt Pending Flag */
    INTR_STATUS_MODEM               = (0 << 1), /* Transmitter Holding Register Empty Interrupt  */
    INTR_STATUS_TRANSMITTER_HOLDING = (1 << 1), /* Received Data Available Interrupt */
    INTR_STATUS_RECEIVE_DATA        = (1 << 2), /* Received Data Available Interrupt */
    INTR_STATUS_RECEIVE_LINE        = (1 << 1) | (1 << 2),  /* Receiver Line Status Interrupt */
    INTR_STATUS_TIME_OUT_PENDING    = (1 << 2) | (1 << 3),  /* Time-out Interrupt Pending (16550 & later) */
    INTR_STATUS_64BYTE_FIFO         = (1 << 5), /* 64 Byte FIFO Enabled (16750 only) */
    INTR_STATUS_NO_FIFO             = (0 << 6), /* No FIFO on chip */
    INTR_STATUS_RESERVED_CONDITION  = (1 << 6), /* Reserved condition */
    INTR_STATUS_FIFO_NOT_FUNC       = (1 << 7), /* FIFO enabled, but not functioning */
    INTR_STATUS_FIFO                = (1 << 6) | (1 << 7),  /* FIFO enabled */
};

enum UartModemControl
{
    MCR_DTR     = 1,        /* Programs -DTR. If set, -DTR is low and the DTR pin of the port goes 'high'. */
    MCR_RTS     = (1 << 1), /* Programs -RTS. dito.  */
    MCR_OUT1    = (1 << 2), /* Programs -OUT1. Normally not used in a PC, but used with some
                            multi-port serial adapters to enable or disable a port. Best
                            thing is to write a '1' to this bit. */
    MCR_OUT2    = (1 << 3), /* Programs -OUT2. If set to 1, interrupts generated by the UART
                            are transferred to the ICU (Interrupt Control Unit) while 0
                            sets the interrupt output of the card to high impedance.
                            (This is PC-only). */
    MCR_LOOPBACK    = (1 << 4),  /* '1': local loopback. All outputs disabled. This is a means of
                            testing the chip: you 'receive' all the data you send. */
};

struct DirectUart
{
    NX_U16 iobase;
    NX_IRQ_Number irqno;

    NX_U16 data;
    NX_U16 divisorLow;
    NX_U16 intrEnable;
    NX_U16 divisorHigh;
    NX_U16 intrIndenty;
    NX_U16 fifo;
    NX_U16 lineCtrl;
    NX_U16 modemCtrl;
    NX_U16 lineStatus;
    NX_U16 modem_status;
    NX_U16 scratch;
};

NX_PRIVATE struct DirectUart DirectUart;
NX_PRIVATE NX_IRQ_DelayWork UartWork;

NX_PRIVATE void UartSent(struct DirectUart *uart, char data)
{
#if UART_SEND_TIMEOUT == 1
    int timeout = 0x100000;
    while (!(IO_In8(uart->lineStatus) & LINE_STATUS_EMPTY_TRANSMITTER_HOLDING) && timeout--)
    {
    }
#else
    while (!(IO_In8(uart->lineStatus) & LINE_STATUS_EMPTY_TRANSMITTER_HOLDING))
    {
    }
#endif
    IO_Out8(uart->data, data);
}

void NX_HalDirectUartPutc(char ch)
{
    if(ch == '\n') {
        UartSent(&DirectUart, '\r');
    }
    UartSent(&DirectUart, ch);
}

NX_INTERFACE void NX_HalConsoleOutChar(char ch)
{
    NX_HalDirectUartPutc(ch);
}

void NX_HalDirectUartInit(void)
{
    struct DirectUart *uart = &DirectUart;
    NX_U16 iobase = UART0_BASE;

    uart->irqno = IRQ_SERIAL1;

    uart->iobase                    = iobase;
    uart->data                      = iobase + 0;
    uart->divisorLow                = iobase + 0;
    uart->intrEnable                = iobase + 1;
    uart->divisorHigh               = iobase + 1;
    uart->intrIndenty               = iobase + 2;
    uart->lineCtrl                  = iobase + 3;
    uart->modemCtrl                 = iobase + 4;
    uart->lineStatus                = iobase + 5;
    uart->modem_status              = iobase + 6;
    uart->scratch                   = iobase + 7;

    IO_Out8(uart->lineCtrl, LINE_DLAB);

    IO_Out8(uart->divisorLow, (BAUD_DIVISOR_DEFAULT) & 0xff);
    IO_Out8(uart->divisorHigh, ((BAUD_DIVISOR_DEFAULT) >> 8) & 0xff);

    IO_Out8(uart->lineCtrl, LINE_WORD_LENGTH_8 |
        LINE_STOP_BIT_1 | LINE_PARITY_NO);

    /* enable recv intr */
    IO_Out8(uart->intrEnable, INTR_RECV_DATA_AVALIABLE |
        INTR_STATUS_CHANGED | INTR_LOW_POWER_MODE);

    /*
     * Set FIFO, open FIFO, clear receive FIFO, clear transmit FIFO Open 64Byte FIFO,
     * interrupt trigger level is 14Byte
     */
    IO_Out8(uart->fifo, FIFO_ENABLE | FIFO_CLEAR_TRANSMIT |
        FIFO_CLEAR_RECEIVE | FIFO_ENABLE_64 |
        FIFO_TRIGGER_14);

    /* IRQs enabled, RTS/DSR set */
    IO_Out8(uart->modemCtrl, MCR_DTR | MCR_RTS | MCR_OUT2);
    IO_Out8(uart->scratch, 0x00);
}

/**
 * default handler
*/
NX_WEAK_SYM void NX_HalDirectUartGetcHandler(char data)
{
    NX_Printf("Deafult uart handler:%x/%c\n", data, data);
}

int NX_HalDirectUartGetc(void)
{
    struct DirectUart *uart = &DirectUart;
    
    int timeout = 100000;
    while (!(IO_In8(uart->lineStatus) & LINE_STATUS_DATA_READY) && timeout--)
    {
    }
    int data = -1;
    if (timeout > 0)
    {
        data = IO_In8(uart->data);
    }
    return data;
}

NX_PRIVATE void UartWorkHandler(void *arg)
{
    int data = NX_HalDirectUartGetc();
    if (data != -1)
    {
        if (NX_HalDirectUartGetcHandler != NX_NULL)
        {
            NX_HalDirectUartGetcHandler(data);
        }
    }
}

NX_PRIVATE NX_Error UartIrqHandler(NX_IRQ_Number irqno, void *arg)
{
    NX_IRQ_DelayWorkHandle(&UartWork);
    return NX_EOK;
}

void NX_HalDirectUartStage2(void)
{
    struct DirectUart *uart = &DirectUart;
    
    NX_ASSERT(NX_IRQ_DelayWorkInit(&UartWork, UartWorkHandler, NX_NULL, NX_IRQ_WORK_NOREENTER) == NX_EOK);
    NX_ASSERT(NX_IRQ_DelayQueueEnter(NX_IRQ_NORMAL_QUEUE, &UartWork) == NX_EOK);

    NX_ASSERT(NX_IRQ_Bind(uart->irqno, UartIrqHandler, NX_NULL, "Uart", 0) == NX_EOK);
    NX_ASSERT(NX_IRQ_Unmask(uart->irqno) == NX_EOK);
}
