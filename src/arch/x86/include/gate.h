/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: x86 gate
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-3      JasonHu           Init
 */

#ifndef __I386_GATE__
#define __I386_GATE__

#include <xbook.h>

#define IDT_LIMIT       0x000007ff
#define IDT_PADDR       0x003F0800

#define IDT_VADDR       (NX_KVADDR_OFFSET + IDT_PADDR)

#define MAX_IDT_NR (IDT_LIMIT/8)

#define OFF(idt, off)    (idt + off)

/* DA: Descriptor Attribute */
#define DA_TASK_GATE        0x85
#define DA_386_CALL_GATE    0x8C
#define DA_386_INTR_GATE    0x8E
#define DA_386_TRAP_GATE    0x8F

#define DA_GATE_DPL0 0
#define DA_GATE_DPL1 1
#define DA_GATE_DPL2 2
#define DA_GATE_DPL3 3

#ifndef __ASSEMBLY__
void CPU_InitGate(void);
#endif

#endif  /* __I386_GATE__ */
