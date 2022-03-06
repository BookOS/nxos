/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: I/O operate 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-9-23      JasonHu           Init
 */

#ifndef __PLATFORM_IO__
#define __PLATFORM_IO__

#include <xbook/types.h>
#include <xbook/defines.h>

NX_INLINE NX_U8 IO_In8(NX_U16 port)
{
    NX_U8 data;
    NX_CASM("inb %w1, %0" : "=a" (data) : "d" (port));
    return data;
}

NX_INLINE NX_U16 IO_In16(NX_U16 port)
{
    NX_U16 data;
    NX_CASM("inw %w1, %0" : "=a" (data) : "d" (port));
    return data;
}

NX_INLINE NX_U32 IO_In32(NX_U16 port)
{
    NX_U32 data;
    NX_CASM("inl %w1, %0" : "=a" (data) : "d" (port));
    return data;
}

NX_INLINE void IO_Out8(NX_U16 port, NX_U8 data)
{
    NX_CASM("outb %0, %w1" : : "a" (data), "d" (port));
}

NX_INLINE void IO_Out16(NX_U16 port, NX_U16 data)
{
    NX_CASM("outw %0, %w1" : : "a" (data), "d" (port));
}

NX_INLINE void IO_Out32(NX_U16 port, NX_U32 data)
{
    NX_CASM("outl %0, %w1" : : "a" (data), "d" (port));
}

#endif  /* __PLATFORM_IO__ */
