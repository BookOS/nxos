/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: process asm 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-05-15     JasonHu           Init
 */

.code32
.text

/*
 * void NX_HalProcessEnterUserMode(frame);
 */
.global NX_HalProcessEnterUserMode
NX_HalProcessEnterUserMode:
    movl 0x4(%esp), %esp
    addl $4,%esp    // skip intr no
    popal
    popl %gs
    popl %fs
    popl %es
    popl %ds
    addl $4, %esp   // skip error code
    iret    // enter to user mode

.align 4
.global __UserThreadReturnCodeBegin
__UserThreadReturnCodeBegin:
    movl %eax, %ebx  // ebx = eax = exit code 
    movl $48, %eax   // eax = 48, SysThreadExit
    int $0x80
.align 4
.global __UserThreadReturnCodeEnd
__UserThreadReturnCodeEnd:
