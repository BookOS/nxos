/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Platform-Level Interrupt Controller(PLIC)
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-4      JasonHu           Init
 */

#include <xbook.h>
#include <plic.h>
#include <regs.h>
#include <sbi.h>
#include <io/irq.h>
#include <utils/log.h>

NX_PRIVATE NX_Error PLIC_SetPriority(NX_IRQ_Number irqno, NX_U32 priority)
{
    if (irqno <= 0 || irqno >= NX_NR_IRQS)
    {
        return NX_EINVAL;
    }

    /* irq priority must be larger than 0 */
    NX_U32 *priorityReg = (NX_U32 *) PLIC_PRIORITY;
    priorityReg += irqno;
    Write32(priorityReg, priority);

    return NX_EOK;
}

NX_Error PLIC_EnableIRQ(NX_U32 hart, NX_IRQ_Number irqno)
{
    if (hart >= NX_MULTI_CORES_NR || irqno <= 0 || irqno >= NX_NR_IRQS)
    {
        return NX_EINVAL;
    }
    
    /* enable irq */
    NX_U32 *enableReg;
    enableReg = (NX_U32 *)PLIC_ENABLE(hart);
    enableReg += irqno / 32;
    NX_U8 off = irqno % 32;
    Write32(enableReg, Read32(enableReg) | (1 << off));

    PLIC_SetPriority(irqno, 1);

    return NX_EOK;
}

NX_Error PLIC_DisableIRQ(NX_U32 hart, NX_IRQ_Number irqno)
{
    if (hart >= NX_MULTI_CORES_NR || irqno <= 0 || irqno >= NX_NR_IRQS)
    {
        return NX_EINVAL;
    }
    
    /* disable irq */
    NX_U32 *enableReg;
    enableReg = (NX_U32 *)PLIC_ENABLE(hart);
    enableReg += irqno / 32;
    NX_U8 off = irqno % 32;
    Write32(enableReg, Read32(enableReg) & ~(1 << off)); 

    PLIC_SetPriority(irqno, 0);

    return NX_EOK;
}

/**
 * ask the PLIC what interrupt we should serve.
 */
NX_IRQ_Number PLIC_Claim(NX_U32 hart)
{
    if (hart >= NX_MULTI_CORES_NR)
    {
        return 0;
    }
    NX_IRQ_Number irq = *(NX_VOLATILE NX_U32 *)PLIC_CLAIM(hart);
    return irq;
}

/**
 * tell the PLIC we've served this IRQ.
 */
NX_Error PLIC_Complete(NX_U32 hart, int irqno)
{
    if (hart >= NX_MULTI_CORES_NR || irqno <= 0 || irqno >= NX_NR_IRQS)
    {
        return NX_EINVAL;
    }
    
    *(NX_U32 *)PLIC_CLAIM(hart) = irqno;

#ifdef CONFIG_NX_PLATFORM_K210
    WriteCSR(sip, ReadCSR(sip) & ~SIP_SSIE); /* clear software pending bit */
    sbi_set_mie();  /* enable machine interrupt after complete */
#endif

    return NX_EOK;
}

void PLIC_Init(NX_Bool bootCore)
{
    if (bootCore == NX_True)
    {
        int hart;
        for (hart = NX_VALID_HARTID_OFFSET; hart < NX_MULTI_CORES_NR; hart++)
        {
            /* priority must be > threshold to trigger an interrupt */
            Write32(PLIC_THRESHOLD(hart), 0);
        }
        /* set all interrupt priority */
        int irqno;
        for (irqno = 1; irqno < NX_NR_IRQS; irqno++)
        {
            PLIC_SetPriority(irqno, 0);
        }
    }
    /* Enable supervisor external interrupts. */
    SetCSR(sie, SIE_SEIE);
}
