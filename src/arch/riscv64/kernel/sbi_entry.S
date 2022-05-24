/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Riscv64 entry 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-1      JasonHu           Init
 * 2022-05-01     JasonHu           support boot from u-boot
 */

    .section .text.start
    .extern __NX_EarlyMain
    .extern __firstBootMagic
    .extern __GetBootHartid

    .globl gStackTop0
    .globl gStackTop1
    .globl gStackTop2
    .globl gStackTop3
    .globl gStackTop4

    .global _Start
_Start:

    csrc sstatus, 0x2 /* disable interrupt */

    /* set global pointer */
.option push
.option norelax
  la gp, __global_pointer$
.option pop
    
    /* check first boot here */
    la t0, __firstBootMagic
    ld t1, (t0)
    li t2, 0x5a5a

    bne t1, t2, _SecondaryBoot
    
    la sp, gStackTop0 # temporary use sp0
    call __GetBootHartid # return hartid to a0

_SecondaryBoot:

    li t0, 0
    beq a0, t0, _SetSP0
    li t0, 1
    beq a0, t0, _SetSP1
    li t0, 2
    beq a0, t0, _SetSP2
    li t0, 3
    beq a0, t0, _SetSP3
    li t0, 4
    beq a0, t0, _SetSP4

    j _EnterMain
    
_SetSP0:
    la sp, gStackTop0
    j _EnterMain

_SetSP1:
    la sp, gStackTop1
    j _EnterMain

_SetSP2:
    la sp, gStackTop2
    j _EnterMain

_SetSP3:
    la sp, gStackTop3
    j _EnterMain

_SetSP4:
    la sp, gStackTop4
    j _EnterMain

_EnterMain:
    csrw sscratch, sp /* first set sscrach as cpu stack here */

    call __NX_EarlyMain

loop:
    j loop

    /* set in data seciton, avoid clear bss to clean stack */
    .section .data.stack
    .align 12

CPU_Stack0:
    .space 8192
gStackTop0:

CPU_Stack1:
    .space 8192
gStackTop1:

CoreStack2:
    .space 8192
gStackTop2:

CoreStack3:
    .space 8192
gStackTop3:

CoreStack4:
    .space 8192
gStackTop4:
