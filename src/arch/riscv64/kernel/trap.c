/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: trap init
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-3      JasonHu           Init
 */

#include <regs.h>
#include <trap.h>
#include <interrupt.h>
#include <clock.h>
#include <plic.h>
#include <regs.h>

#define NX_LOG_NAME "Trap"
#define NX_LOG_LEVEL NX_LOG_INFO
#include <utils/log.h>
#include <xbook/debug.h>

#include <sched/thread.h>
#include <sched/smp.h>
#include <utils/memory.h>

 /* (syscall) Environment call from U-mode */
#define RISCV_SYSCALL_EXCEPTION 8

/* trap name for riscv */
NX_PRIVATE const char *InterruptName[] =
{
    "User Software Interrupt",
    "Supervisor Software Interrupt",
    "Reversed-2",
    "Reversed-3",
    "User Timer Interrupt",
    "Supervisor Timer Interrupt",
    "Reversed-6",
    "Reversed-7",
    "User External Interrupt",
    "Supervisor External Interrupt",
    "Reserved-10",
    "Reserved-11",
};

NX_PRIVATE const char *ExceptionName[] =
{
    "Instruction Address Misaligned",
    "Instruction Access Fault",
    "Illegal Instruction",
    "Breakpoint",
    "Load Address Misaligned",
    "Load Access Fault",
    "Store/AMO Address Misaligned",
    "Store/AMO Access Fault",
    "Environment call from U-mode",
    "Environment call from S-mode",
    "Reserved-10",
    "Reserved-11",
    "Instruction Page Fault",
    "Load Page Fault",
    "Reserved-14",
    "Store/AMO Page Fault"
};

NX_PUBLIC void TrapFrameDump(HAL_TrapFrame *frame)
{
    NX_LOG_RAW("------------ Trap frame Dump ------------\n");
    NX_LOG_RAW("Function Registers:\n");
    NX_LOG_RAW("\tra(x1) = 0x%p\n", frame->ra);
    NX_LOG_RAW("\tuser_sp(x2) = 0x%p\n", frame->sp);
    NX_LOG_RAW("\tgp(x3) = 0x%p\n", frame->gp);
    NX_LOG_RAW("\ttp(x4) = 0x%p\n", frame->tp);
    NX_LOG_RAW("Temporary Registers:\n");
    NX_LOG_RAW("\tt0(x5) = 0x%p\n", frame->t0);
    NX_LOG_RAW("\tt1(x6) = 0x%p\n", frame->t1);
    NX_LOG_RAW("\tt2(x7) = 0x%p\n", frame->t2);
    NX_LOG_RAW("\tt3(x28) = 0x%p\n", frame->t3);
    NX_LOG_RAW("\tt4(x29) = 0x%p\n", frame->t4);
    NX_LOG_RAW("\tt5(x30) = 0x%p\n", frame->t5);
    NX_LOG_RAW("\tt6(x31) = 0x%p\n", frame->t6);
    NX_LOG_RAW("Saved Registers:\n");
    NX_LOG_RAW("\ts0/fp(x8) = 0x%p\n", frame->s0);
    NX_LOG_RAW("\ts1(x9) = 0x%p\n", frame->s1);
    NX_LOG_RAW("\ts2(x18) = 0x%p\n", frame->s2);
    NX_LOG_RAW("\ts3(x19) = 0x%p\n", frame->s3);
    NX_LOG_RAW("\ts4(x20) = 0x%p\n", frame->s4);
    NX_LOG_RAW("\ts5(x21) = 0x%p\n", frame->s5);
    NX_LOG_RAW("\ts6(x22) = 0x%p\n", frame->s6);
    NX_LOG_RAW("\ts7(x23) = 0x%p\n", frame->s7);
    NX_LOG_RAW("\ts8(x24) = 0x%p\n", frame->s8);
    NX_LOG_RAW("\ts9(x25) = 0x%p\n", frame->s9);
    NX_LOG_RAW("\ts10(x26) = 0x%p\n", frame->s10);
    NX_LOG_RAW("\ts11(x27) = 0x%p\n", frame->s11);
    NX_LOG_RAW("Function Arguments Registers:\n");
    NX_LOG_RAW("\ta0(x10) = 0x%p\n", frame->a0);
    NX_LOG_RAW("\ta1(x11) = 0x%p\n", frame->a1);
    NX_LOG_RAW("\ta2(x12) = 0x%p\n", frame->a2);
    NX_LOG_RAW("\ta3(x13) = 0x%p\n", frame->a3);
    NX_LOG_RAW("\ta4(x14) = 0x%p\n", frame->a4);
    NX_LOG_RAW("\ta5(x15) = 0x%p\n", frame->a5);
    NX_LOG_RAW("\ta6(x16) = 0x%p\n", frame->a6);
    NX_LOG_RAW("\ta7(x17) = 0x%p\n", frame->a7);
    NX_LOG_RAW("sstatus = 0x%p\n", frame->sstatus);
    NX_LOG_RAW("\t%s\n", (frame->sstatus & SSTATUS_SIE) ? "Supervisor Interrupt Enabled" : "Supervisor Interrupt Disabled");
    NX_LOG_RAW("\t%s\n", (frame->sstatus & SSTATUS_SPIE) ? "Last Time Supervisor Interrupt Enabled" : "Last Time Supervisor Interrupt Disabled");
    NX_LOG_RAW("\t%s\n", (frame->sstatus & SSTATUS_SPP) ? "Last Privilege is Supervisor Mode" : "Last Privilege is User Mode");
    NX_LOG_RAW("\t%s\n", (frame->sstatus & SSTATUS_FS) ? "FPU Enabled" : "FPU not Enabled");
    
    NX_Size satp = ReadCSR(satp);
    NX_LOG_RAW("satp = 0x%p\n", satp);
    
    NX_LOG_RAW("\tCurrent PPN = 0x%p\n", (satp & ((1UL << 28) - 1)));
    NX_LOG_RAW("\tCurrent ASID = 0x%p\n", (satp >> 44) & ((1UL << 16) - 1));

    const char *mmuMode;
    switch ((satp >> 60) & 0xF)
    {
    case 0:
        mmuMode = "Bare - No translation or protection";
        break;
    case 8:
        mmuMode = "Sv39 - Page-based 39-bit virtual addressing";
        break;
    case 9:
        mmuMode = "Sv48 - Page-based 48-bit virtual addressing";
        break;
    case 10:
        mmuMode = "Sv57 - Page-based 57-bit virtual addressing";
        break;
    case 11:
        mmuMode = "Sv64 - Page-based 64-bit virtual addressing";
        break;
    default:
        mmuMode = " - Reserved";
        break;
    }

    NX_LOG_RAW("\tAddress Transport Mode = %s\n", mmuMode);
    NX_LOG_RAW("------------ Trap frame Dump Done ------------\n");
}

NX_IMPORT NX_Addr TrapEntry0;
NX_IMPORT NX_Addr TrapEntry1;
NX_IMPORT NX_Addr TrapEntry2;
NX_IMPORT NX_Addr TrapEntry3;

NX_PUBLIC void CPU_InitTrap(NX_UArch coreId)
{
    NX_Addr *trapEntry = 0;
    switch (coreId)
    {
    case 0:
        trapEntry = &TrapEntry0;
        break;
    case 1:
        trapEntry = &TrapEntry1;
        break;
    case 2:
        trapEntry = &TrapEntry2;
        break;
    case 3:
        trapEntry = &TrapEntry3;
        break;
    default:
        return;
    }
    /* set trap entry */
    WriteCSR(stvec, trapEntry);

    /* Enable soft interrupt */
    SetCSR(sie, SIE_SSIE);
}

NX_IMPORT void HAL_ProcessSyscallDispatch(HAL_TrapFrame *frame);

NX_PUBLIC void TrapDispatch(HAL_TrapFrame *frame)
{
    NX_U64 cause = ReadCSR(scause);
    NX_U64 stval = ReadCSR(stval);

    const char *msg = NX_NULL;
    NX_UArch id = cause & ((1UL << (RISCV_XLEN - 2)) - 1);

    /* supervisor external interrupt */
#ifdef CONFIG_NX_PLATFROM_K210
    /* 
     * on k210, supervisor software interrupt is used 
	 * in alternative to supervisor external interrupt, 
	 * which is not available on k210. 
	 */
    if ((SCAUSE_INTERRUPT | SCAUSE_S_SOFTWARE_INTR) == cause && 
        SCAUSE_S_EXTERNAL_INTR == stval)
#else
    if ((SCAUSE_INTERRUPT & cause) && 
        SCAUSE_S_EXTERNAL_INTR == (cause & 0xff))
#endif
    {    
        NX_IRQ_Number irqno = PLIC_Claim(NX_SMP_GetBootCore());
        if (irqno != 0)
        {
            NX_IRQ_Handle(irqno);
        }
        return;
    }
    else if ((SCAUSE_INTERRUPT | SCAUSE_S_TIMER_INTR) == cause)
    {
        /* supervisor timer */
        HAL_ClockHandler();
        return;
    }
    else if (SCAUSE_INTERRUPT & cause)
    {
        if(id < sizeof(InterruptName) / sizeof(const char *))
        {
            msg = InterruptName[id];
        }
        else
        {
            msg = "Unknown Interrupt";
        }
        NX_LOG_E("Unhandled Interrupt %ld:%s", id, msg);
    }
    else /* exceptions */
    {
        if (id == RISCV_SYSCALL_EXCEPTION)
        {
            /* NOTICE: need enable interrupt while dispatch syscall */
            NX_IRQ_Enable();
            HAL_ProcessSyscallDispatch(frame);
            NX_IRQ_Disable();
            return;
        }

        if(id < sizeof(ExceptionName) / sizeof(const char *))
        {
            msg = ExceptionName[id];
        }
        else
        {
            msg = "Unknown Exception";
        }
        NX_LOG_E("Unhandled Exception %ld:%s", id, msg);
    }

    NX_LOG_E("scause:0x%p, stval:0x%p, sepc:0x%p", cause, stval, frame->epc);
    TrapFrameDump(frame);
    while(1);
}

/**
 * switch CPU stack to thread stack, thread stack maybe stack top or stack middle.
 */
NX_PUBLIC NX_U8 *TrapSwitchStack(HAL_TrapFrame *frame)
{
    NX_UArch sstatus = ReadCSR(sstatus);
    NX_U8 *sp;
    if ((sstatus & SSTATUS_SPP)) /* trap from supervisor */
    {
        sp = (NX_U8 *)ReadCSR(sscratch); /* read from sscratch, it saved old sp in kernel */
    }
    else    /* trap from user */
    {
        NX_LOG_D("from user:%p, sstatus:%p", frame, sstatus);
        NX_Thread *thread = NX_ThreadSelf();
        sp = (NX_U8 *)(thread->stackBase + thread->stackSize);
    }
    sp -= sizeof(HAL_TrapFrame);
    /* copy TrapFrame to new stack */
    NX_MemCopy((void *)sp, frame, sizeof(HAL_TrapFrame));
    return sp; 
}
