/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: NX_Thread context 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-01     JasonHu           Init
 */

.text

#define __ASSEMBLY__
#include <context.h>
#include <regs.h>

/**
 * void NX_HalContextSwitchNext(NX_Addr nextSP);
 */
.globl NX_HalContextSwitchNext
NX_HalContextSwitchNext:
    LOAD sp, (a0)   /* sp = *nextSP */

    RESTORE_CONTEXT
    sret

/**
 * void NX_HalContextSwitchPrevNext(NX_Addr prevSP, NX_Addr nextSP);
 */
.globl NX_HalContextSwitchPrevNext
NX_HalContextSwitchPrevNext:
    mv t0, sp

    /**
     * set as supervisor mode and enable interupt 
     * make sure kernel mode thread do context switch
     */
    li t1, SSTATUS_SPP | SSTATUS_SPIE | SSTATUS_SUM
    csrs sstatus, t1

    /**
     * set `sepc` as `ra`, `ra` saved return address of NX_HalContextSwitchPrevNext
     * when do `sret`, will return from NX_HalContextSwitchPrevNext
     */
    csrw sepc, ra
    
    /* save context to stack */
    SAVE_CONTEXT

    /* save old sp to stack, instead the sp in sscratch */
    STORE t0, CTX_SP_OFF * REGBYTES(sp)

    /* save sp to from thread, *from = `sp` */
    STORE sp, (a0)

    LOAD sp, (a1)   /* sp = *nextSP */

    /* restore context from stack */
    RESTORE_CONTEXT
    
    /* supervisor return, `pc` = `sepc` */
    sret
