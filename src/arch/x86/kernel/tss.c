/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: task status segment
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-9-17      JasonHu           Init
 */

#include <tss.h>
#include <segment.h>
#include <regs.h>
#include <utils/memory.h>

NX_PRIVATE struct CPU_TSS tss;

struct CPU_TSS *CPU_GetTSS(void)
{
    return &tss;
}

void CPU_SetTssStack(NX_UArch top)
{
    tss.esp0 = top; /* esp0 is kernel mode */
}

void CPU_InitTSS(void)
{
    NX_MemZero(&tss, sizeof(struct CPU_TSS));
    tss.esp0 = KERNEL_STACK_TOP;
    tss.ss0 = KERNEL_DATA_SEL;
    tss.iobase = sizeof(struct CPU_TSS);

    CPU_LoadTR(KERNEL_TSS_SEL);
}
