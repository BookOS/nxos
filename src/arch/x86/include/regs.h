/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: arch registers 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-22      JasonHu           Init
 */

#ifndef __PLATFORM_REGS__
#define __PLATFORM_REGS__

#include <base/types.h>
#include <base/defines.h>

#define EFLAGS_MBS    (1 << 1)
#define EFLAGS_IF_1   (1 << 9)
#define EFLAGS_IF_0 0
#define EFLAGS_IOPL_3 (3 << 12)
#define EFLAGS_IOPL_1 (1 << 12)
#define EFLAGS_IOPL_0 (0 << 12)

#define EFLAGS_IF (EFLAGS_IF_1)

/* cr0 bit 31 is page enable bit, 1: enable MMU, 0: disable MMU */
#define CR0_PG  (1 << 31)

/* cr0 bit 31 is page enable bit, 1: enable MMU, 0: disable MMU */
#define CR0_PG  (1 << 31)

NX_INLINE void CPU_LoadTR(NX_U32 selector)
{
    NX_CASM("ltr %w0" : : "q" (selector));
}

NX_INLINE void CPU_WriteCR3(NX_U32 val)
{
    NX_CASM("movl %0, %%cr3" : : "a" (val));
}

NX_INLINE NX_U32 CPU_ReadCR3(void)
{
    NX_U32 val;
    NX_CASM("movl %%cr3, %0\n\t": "=a" (val));
    return val;
}

NX_INLINE NX_U32 CPU_ReadCR2(void)
{
    NX_U32 val;
    NX_CASM("movl %%cr2, %0\n\t": "=a" (val));
    return val;
}

NX_INLINE NX_U32 CPU_ReadCR0(void)
{
    NX_U32 val;
    NX_CASM("movl %%cr0, %0\n\t": "=a" (val));
    return val;
}

NX_INLINE void CPU_WriteCR0(NX_U32 val)
{
    NX_CASM("movl %0, %%cr0\n\t": :"a" (val));
}

NX_INLINE NX_U32 CPU_ReadESP(void)
{
    NX_U32 sp;
    NX_CASM("movl %%esp, %0\n\t": "=a" (sp));
    return sp;
}

#endif  /* __PLATFORM_REGS__ */
