/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Riscv64 entry 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-1      JasonHu           Init
 */

    .section .text.start
    .extern __NX_EarlyMain

    .globl CPU_StackTop0
    .globl CPU_StackTop1
    .globl CPU_StackTop2
    .globl CPU_StackTop3

    .global _Start
_Start:
    li t0, 0
    beq a0, t0, _SetSP0
    li t0, 1
    beq a0, t0, _SetSP1
    li t0, 2
    beq a0, t0, _SetSP2
    li t0, 3
    beq a0, t0, _SetSP3

    j _EnterMain
    
_SetSP0:
    la sp, CPU_StackTop0
    j _EnterMain

_SetSP1:
    la sp, CPU_StackTop1
    j _EnterMain

_SetSP2:
    la sp, CPU_StackTop2
    j _EnterMain

_SetSP3:
    la sp, CPU_StackTop3
    j _EnterMain

_EnterMain:
    csrw sscratch, sp /* first set sscrach as cpu stack here */
    csrc sstatus, 0x2 /* disable interrupt */

.option push
.option norelax
  la gp, __global_pointer$
.option pop
    
    call __NX_EarlyMain

loop:
    j loop

    /* set in data seciton, avoid clear bss to clean stack */
    .section .data.stack
    .align 12

CPU_Stack0:
    .space 8192
CPU_StackTop0:

CPU_Stack1:
    .space 8192
CPU_StackTop1:

CoreStack2:
    .space 8192
CPU_StackTop2:

CoreStack3:
    .space 8192
CPU_StackTop3:
