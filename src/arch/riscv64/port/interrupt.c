/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: interrupt manage
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-31     JasonHu           Init
 */

#include <nxos.h>
#include <regs.h>
#include <base/irq.h>
#include <plic.h>
#include <base/smp.h>

NX_PRIVATE NX_Error NX_HalIrqUnmask(NX_IRQ_Number irqno)
{
    if (irqno < 0 || irqno >= NX_NR_IRQS)
    {
        return NX_EINVAL;
    }

    return PLIC_EnableIRQ(NX_SMP_GetBootCore(), irqno);
}

NX_PRIVATE NX_Error NX_HalIrqMask(NX_IRQ_Number irqno)
{
    if (irqno < 0 || irqno >= NX_NR_IRQS)
    {
        return NX_EINVAL;
    }

    return PLIC_DisableIRQ(NX_SMP_GetBootCore(), irqno);
}

NX_PRIVATE NX_Error NX_HalIrqAck(NX_IRQ_Number irqno)
{
    if (irqno < 0 || irqno >= NX_NR_IRQS)
    {
        return NX_EINVAL;
    }
    
    return PLIC_Complete(NX_SMP_GetBootCore(), irqno);
}

NX_PRIVATE void NX_HalIrqEnable(void)
{
    WriteCSR(sstatus, ReadCSR(sstatus) | SSTATUS_SIE);
}

NX_PRIVATE void NX_HalIrqDisable(void)
{
    WriteCSR(sstatus, ReadCSR(sstatus) & ~SSTATUS_SIE);
}

NX_PRIVATE NX_UArch NX_HalIrqSaveLevel(void)
{
    NX_UArch level = 0;
    level = ReadCSR(sstatus);
    WriteCSR(sstatus, level & ~SSTATUS_SIE);
    return level & SSTATUS_SIE;
}

NX_PRIVATE void NX_HalIrqRestoreLevel(NX_UArch level)
{
    WriteCSR(sstatus, ReadCSR(sstatus) | (level & SSTATUS_SIE));
}

NX_INTERFACE NX_IRQ_Controller NX_IRQ_ControllerInterface = 
{
    .unmask = NX_HalIrqUnmask,
    .mask = NX_HalIrqMask,
    .ack = NX_HalIrqAck,
    .enable = NX_HalIrqEnable,
    .disable = NX_HalIrqDisable,
    .saveLevel = NX_HalIrqSaveLevel,
    .restoreLevel = NX_HalIrqRestoreLevel,
};
