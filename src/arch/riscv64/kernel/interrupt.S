/**
 * Copyright (c) 2018-2022, NXOS Development Team
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

.extern gStackTop0
.extern gStackTop1
.extern gStackTop2
.extern gStackTop3
.extern gStackTop4

.globl gTrapEntry0
.globl gTrapEntry1
.globl gTrapEntry2
.globl gTrapEntry3
.globl gTrapEntry4

.align 2 # TrapEntry must aligin with 4 byte
gTrapEntry0:
    /* save sp to sscratch reg, sscratch saved old sp from user/kernel */
    csrrw sp, sscratch, sp    
    /* switch to cpu stack as temp stack */
    la sp, gStackTop0
    j TrapEntry

.align 2
gTrapEntry1:
    csrrw sp, sscratch, sp    
    la sp, gStackTop1
    j TrapEntry

.align 2
gTrapEntry2:
    csrrw sp, sscratch, sp    
    la sp, gStackTop2
    j TrapEntry

.align 2
gTrapEntry3:
    csrrw sp, sscratch, sp    
    la sp, gStackTop3
    j TrapEntry

.align 2
gTrapEntry4:
    csrrw sp, sscratch, sp    
    la sp, gStackTop4
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
