/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: process asm
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-05-20     JasonHu           Init
 */

#define __ASSEMBLY__
#include <regs.h>

/*
 * void NX_HalProcessEnterUserMode(args, text, userStack, returnAddr);
 */
.align 3
.global NX_HalProcessEnterUserMode
NX_HalProcessEnterUserMode:
    /* set as user mode, close interrupt */
    li t0, SSTATUS_SPP | SSTATUS_SIE
    csrc sstatus, t0

    /* enable interrupt when return to user mode */
    li t0, SSTATUS_SPIE
    csrs sstatus, t0

    csrw sepc, a1   /* set text */
    mv sp, a2       /* set sp */
    mv ra, a3       /* set return addr */
    sret            /* enter user mode */

.align 3
.global __UserThreadReturnCodeBegin
__UserThreadReturnCodeBegin:
    li a0, 0    // a0 = exit code 
    li a7, 48   // a7 = 48, SysThreadExit
    ecall

.align 3
.global __UserThreadReturnCodeEnd
__UserThreadReturnCodeEnd:
