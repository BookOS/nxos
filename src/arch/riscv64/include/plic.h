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

#ifndef __RISCV_PLIC__
#define __RISCV_PLIC__

#include <nxos.h>
#include <riscv.h>
#include <base/irq.h>

#define PLIC_ADDR               RISCV_PLIC_PADDR

/* interrupt priority */
#define PLIC_PRIORITY           (PLIC_ADDR + 0x0)
/* interrupt pending */
#define PLIC_PENDING            (PLIC_ADDR + 0x1000)
#define PLIC_MEMSZ0             0x4000

#ifdef CONFIG_NX_PLATFORM_K210

/* interrupt enable for each hart */
#define __PLIC_ENABLE(hart)      (PLIC_ADDR + 0x2000 + (hart) * 0x80)
/* priority threshold for each hart */
#define __PLIC_THRESHOLD(hart)   (PLIC_ADDR + 0x200000 + (hart) * 0x1000)
/* claim for each hart */
#define __PLIC_CLAIM(hart)       (PLIC_ADDR + 0x200004 + (hart) * 0x1000)

#define PLIC_ENABLE(hart)       __PLIC_ENABLE(hart)
#define PLIC_THRESHOLD(hart)    __PLIC_THRESHOLD(hart)
#define PLIC_CLAIM(hart)        __PLIC_CLAIM(hart)
#define PLIC_MEMSZ1             0x4000

#elif defined(CONFIG_NX_PLATFORM_HIFIVE_UNMACHED)
/* machine interrupt enable for each hart */
#define __PLIC_MENABLE(hart)      (PLIC_ADDR + 0x2080 + ((hart) - 1) * 0x100)
/* supervisor interrupt enable for each hart */
#define __PLIC_SENABLE(hart)      (PLIC_ADDR + 0x2000 + (hart) * 0x100)
/* machine priority threshold for each hart */
#define __PLIC_MTHRESHOLD(hart)   (PLIC_ADDR + 0x201000 + ((hart) - 1) * 0x2000)
/* supervisor priority threshold for each hart */
#define __PLIC_STHRESHOLD(hart)   (PLIC_ADDR + 0x200000 + (hart) * 0x2000)
/* machine claim for each hart */
#define __PLIC_MCLAIM(hart)       (PLIC_ADDR + 0x201004 + ((hart) - 1) * 0x2000)
/* supervisor claim for each hart */
#define __PLIC_SCLAIM(hart)       (PLIC_ADDR + 0x200004 + (hart) * 0x2000)

#define PLIC_ENABLE(hart)       __PLIC_SENABLE(hart)
#define PLIC_THRESHOLD(hart)    __PLIC_STHRESHOLD(hart)
#define PLIC_CLAIM(hart)        __PLIC_SCLAIM(hart)
#define PLIC_MEMSZ1             (5 * 0x2000 + 0x1000)

#else
/* machine interrupt enable for each hart */
#define __PLIC_MENABLE(hart)      (PLIC_ADDR + 0x2000 + (hart) * 0x100)
/* supervisor interrupt enable for each hart */
#define __PLIC_SENABLE(hart)      (PLIC_ADDR + 0x2080 + (hart) * 0x100)
/* machine priority threshold for each hart */
#define __PLIC_MTHRESHOLD(hart)   (PLIC_ADDR + 0x200000 + (hart) * 0x2000)
/* supervisor priority threshold for each hart */
#define __PLIC_STHRESHOLD(hart)   (PLIC_ADDR + 0x201000 + (hart) * 0x2000)
/* machine claim for each hart */
#define __PLIC_MCLAIM(hart)       (PLIC_ADDR + 0x200004 + (hart) * 0x2000)
/* supervisor claim for each hart */
#define __PLIC_SCLAIM(hart)       (PLIC_ADDR + 0x201004 + (hart) * 0x2000)

#define PLIC_ENABLE(hart)       __PLIC_SENABLE(hart)
#define PLIC_THRESHOLD(hart)    __PLIC_STHRESHOLD(hart)
#define PLIC_CLAIM(hart)        __PLIC_SCLAIM(hart)
#define PLIC_MEMSZ0             0x4000
#define PLIC_MEMSZ1             (4 * 0x2000 + 0x2000)

#endif

NX_Error PLIC_EnableIRQ(NX_U32 hart, NX_IRQ_Number irqno);
NX_Error PLIC_DisableIRQ(NX_U32 hart, NX_IRQ_Number irqno);
NX_IRQ_Number PLIC_Claim(NX_U32 hart);
NX_Error PLIC_Complete(NX_U32 hart, int irqno);
void PLIC_Init(NX_Bool bootCore);

#endif  /* __RISCV_PLIC__ */
