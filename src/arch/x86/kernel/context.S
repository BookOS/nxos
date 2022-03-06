/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: NX_Thread context 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-16     JasonHu           Init
 */

.code32
.text

/**
 * void NX_HalContextSwitchNext(NX_Addr nextSP);
 */
.globl NX_HalContextSwitchNext
NX_HalContextSwitchNext:
    movl 0x4(%esp), %eax    // get thread "nextSP"
    movl (%eax), %esp       // restore sp
    
    popl %ebp
    popl %ebx
    popl %edi
    popl %esi
    ret

/**
 * void NX_HalContextSwitchPrevNext(NX_Addr prevSP, NX_Addr nextSP);
 */
.globl NX_HalContextSwitchPrevNext
NX_HalContextSwitchPrevNext:
    pushl %esi
    pushl %edi
    pushl %ebx
    pushl %ebp

    movl 0x14(%esp), %eax   // get "prevSP"
    movl %esp, (%eax)       // save sp

    movl 0x18(%esp), %eax   // get "nextSP"
    movl (%eax), %esp       // restore sp

    popl %ebp
    popl %ebx
    popl %edi
    popl %esi
    ret
