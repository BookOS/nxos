/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: X86 Gate 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-9-17      JasonHu           Init
 */

#include <gate.h>
#include <segment.h>
#include <interrupt.h>

struct CPU_Gate
{
    NX_U16 offsetLow, selector;
    NX_U8 datacount;
    NX_U8 attributes;      /* P(1) DPL(2) DT(1) TYPE(4) */
    NX_U16 offsetHigh;
};

typedef void (*TrapHandler)(void);

void CPU_TrapEntry0x00(void);
void CPU_TrapEntry0x01(void);
void CPU_TrapEntry0x02(void);
void CPU_TrapEntry0x03(void);
void CPU_TrapEntry0x04(void);
void CPU_TrapEntry0x05(void);
void CPU_TrapEntry0x06(void);
void CPU_TrapEntry0x07(void);
void CPU_TrapEntry0x08(void);
void CPU_TrapEntry0x09(void);
void CPU_TrapEntry0x0a(void);
void CPU_TrapEntry0x0b(void);
void CPU_TrapEntry0x0c(void);
void CPU_TrapEntry0x0d(void);
void CPU_TrapEntry0x0e(void);
void CPU_TrapEntry0x0f(void);
void CPU_TrapEntry0x10(void);
void CPU_TrapEntry0x11(void);
void CPU_TrapEntry0x12(void);
void CPU_TrapEntry0x13(void);
void CPU_TrapEntry0x14(void);
void CPU_TrapEntry0x15(void);
void CPU_TrapEntry0x16(void);
void CPU_TrapEntry0x17(void);
void CPU_TrapEntry0x18(void);
void CPU_TrapEntry0x19(void);
void CPU_TrapEntry0x1a(void);
void CPU_TrapEntry0x1b(void);
void CPU_TrapEntry0x1c(void);
void CPU_TrapEntry0x1d(void);
void CPU_TrapEntry0x1e(void);
void CPU_TrapEntry0x1f(void);

void CPU_TrapEntry0x20(void);
void CPU_TrapEntry0x21(void);
void CPU_TrapEntry0x22(void);
void CPU_TrapEntry0x23(void);
void CPU_TrapEntry0x24(void);
void CPU_TrapEntry0x25(void);
void CPU_TrapEntry0x26(void);
void CPU_TrapEntry0x27(void);
void CPU_TrapEntry0x28(void);
void CPU_TrapEntry0x29(void);
void CPU_TrapEntry0x2a(void);
void CPU_TrapEntry0x2b(void);
void CPU_TrapEntry0x2c(void);
void CPU_TrapEntry0x2d(void);
void CPU_TrapEntry0x2e(void);
void CPU_TrapEntry0x2f(void);

void CPU_SyscallEntry(void);
void CPU_LoadIDT(NX_UArch NX_Size, NX_UArch idtr);    

NX_PRIVATE void SetGate(struct CPU_Gate *gate, TrapHandler handler,
                     NX_U32 selector, NX_U32 attributes, NX_U8 privilege)
{
    NX_UArch offset = (NX_UArch) handler;
    gate->offsetLow  = offset & 0xffff;
    gate->selector    = selector;
    gate->attributes  = attributes | (privilege << 5);
    gate->datacount   = 0;
    gate->offsetHigh = (offset >> 16) & 0xffff;
}

void CPU_InitGate(void)
{
    struct CPU_Gate *idt = (struct CPU_Gate *) (IDT_VADDR);

    int i;
    for (i = 0; i < MAX_IDT_NR; i++) {
        SetGate(OFF(idt, i), 0, 0, 0, 0);
    }
    SetGate(OFF(idt, EXCEPTION_BASE), CPU_TrapEntry0x00, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+1), CPU_TrapEntry0x01, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+2), CPU_TrapEntry0x02, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+3), CPU_TrapEntry0x03, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+4), CPU_TrapEntry0x04, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+5), CPU_TrapEntry0x05, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+6), CPU_TrapEntry0x06, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+7), CPU_TrapEntry0x07, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+8), CPU_TrapEntry0x08, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+9), CPU_TrapEntry0x09, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+10), CPU_TrapEntry0x0a, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+11), CPU_TrapEntry0x0b, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+12), CPU_TrapEntry0x0c, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+13), CPU_TrapEntry0x0d, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+14), CPU_TrapEntry0x0e, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+15), CPU_TrapEntry0x0f, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+16), CPU_TrapEntry0x10, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+17), CPU_TrapEntry0x11, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+18), CPU_TrapEntry0x12, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+19), CPU_TrapEntry0x13, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+20), CPU_TrapEntry0x14, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+21), CPU_TrapEntry0x15, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+22), CPU_TrapEntry0x16, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+23), CPU_TrapEntry0x17, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+24), CPU_TrapEntry0x18, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+25), CPU_TrapEntry0x19, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+26), CPU_TrapEntry0x1a, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+27), CPU_TrapEntry0x1b, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+28), CPU_TrapEntry0x1c, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+29), CPU_TrapEntry0x1d, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+30), CPU_TrapEntry0x1e, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXCEPTION_BASE+31), CPU_TrapEntry0x1f, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);

    SetGate(OFF(idt, EXTERNAL_BASE), CPU_TrapEntry0x20, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+1), CPU_TrapEntry0x21, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+2), CPU_TrapEntry0x22, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+3), CPU_TrapEntry0x23, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+4), CPU_TrapEntry0x24, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+5), CPU_TrapEntry0x25, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+6), CPU_TrapEntry0x26, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+7), CPU_TrapEntry0x27, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+8), CPU_TrapEntry0x28, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+9), CPU_TrapEntry0x29, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+10), CPU_TrapEntry0x2a, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+11), CPU_TrapEntry0x2b, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+12), CPU_TrapEntry0x2c, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+13), CPU_TrapEntry0x2d, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+14), CPU_TrapEntry0x2e, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);
    SetGate(OFF(idt, EXTERNAL_BASE+15), CPU_TrapEntry0x2f, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL0);

    SetGate(OFF(idt, SYSCALL_BASE), CPU_SyscallEntry, KERNEL_CODE_SEL, DA_386_INTR_GATE, DA_GATE_DPL3);
    
    CPU_LoadIDT(IDT_LIMIT, IDT_VADDR);
}
