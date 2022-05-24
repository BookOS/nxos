/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: x86 tss
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-3      JasonHu           Init
 */

#ifndef __I386_TSS__
#define __I386_TSS__

#include <nxos.h>

#define KERNEL_STACK_TOP_PHY 0x9f000
#define KERNEL_STACK_TOP (NX_KVADDR_OFFSET + KERNEL_STACK_TOP_PHY)

struct CPU_TSS
{
    NX_U32 backlink;
    NX_U32 esp0;
    NX_U32 ss0;
    NX_U32 esp1;
    NX_U32 ss1;
    NX_U32 esp2;
    NX_U32 ss2;
    NX_U32 cr3;
    NX_U32 eip;
    NX_U32 eflags;
    NX_U32 eax;
    NX_U32 ecx;
    NX_U32 edx;
    NX_U32 ebx;
    NX_U32 esp;
    NX_U32 ebp;
    NX_U32 esi;
    NX_U32 edi;
    NX_U32 es;
    NX_U32 cs;
    NX_U32 ss;
    NX_U32 ds;
    NX_U32 fs;
    NX_U32 gs;
    NX_U32 ldtr;
    NX_U32 trap;
    NX_U32 iobase;
};

void CPU_InitTSS(void);
struct CPU_TSS *CPU_GetTSS(void);
void CPU_SetTssStack(NX_UArch top);

#endif  /* __I386_TSS__ */
