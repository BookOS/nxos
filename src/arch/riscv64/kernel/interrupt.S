/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Interrupt
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-03     JasonHu           Init
 */

#define __ASSEMBLY__
#include <context.h>
#include <regs.h>

.text

.extern NX_IRQ_DelayQueueCheck
.extern TrapDispatch
.extern TrapSwitchStack
.extern NX_ReSchedCheck

.extern CPU_StackTop0
.extern CPU_StackTop1
.extern CPU_StackTop2
.extern CPU_StackTop3

.globl TrapEntry0
.globl TrapEntry1
.globl TrapEntry2
.globl TrapEntry3

.align 2 # TrapEntry must aligin with 4 byte
TrapEntry0:
    /* save sp to sscratch reg, sscratch saved old sp from user/kernel */
    csrrw sp, sscratch, sp    
    /* switch to cpu stack as temp stack */
    la sp, CPU_StackTop0
    j TrapEntry

.align 2
TrapEntry1:
    csrrw sp, sscratch, sp    
    la sp, CPU_StackTop1
    j TrapEntry

.align 2
TrapEntry2:
    csrrw sp, sscratch, sp    
    la sp, CPU_StackTop2
    j TrapEntry

.align 2
TrapEntry3:
    csrrw sp, sscratch, sp    
    la sp, CPU_StackTop3
    j TrapEntry

TrapEntry:

    /* save context to cpu stack */
    SAVE_CONTEXT

    /* switch stack from CPU stack to thread stack */
    mv a0, sp
    call TrapSwitchStack
    mv sp, a0 /* a0 saved new stack */

    /* call handler */
    mv a0, sp
    call TrapDispatch
    
    /* only external IRQ and timer IRQ check IRQ queue */
    csrr t0, scause
    li t1, SCAUSE_INTERRUPT
    and t0, t0, t1
    beqz t0, TrapReturn /* No interrupt bit */

    /* do IRQ queue check */
    call NX_IRQ_DelayQueueCheck

    /* check sched */
    call NX_ReSchedCheck

    tail TrapReturn

.globl TrapReturn
TrapReturn:
    
    /* restore context will restore sp */
    RESTORE_CONTEXT
    /* return from supervisor call */
    sret

/*
 * void HAL_ProcessEnterUserMode(args, text, userStack);
 */
.global HAL_ProcessEnterUserMode
HAL_ProcessEnterUserMode:
    /* set as user mode, close interrupt */
    li t0, SSTATUS_SPP | SSTATUS_SIE
    csrc sstatus, t0

    /* enable interrupt when return to user mode */
    li t0, SSTATUS_SPIE
    csrs sstatus, t0

    csrw sepc, a1   /* set text */
    mv sp, a2       /* set sp */
    sret            /* enter user mode */
