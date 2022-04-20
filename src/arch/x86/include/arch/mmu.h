/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Memory Manage Unite 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-20     JasonHu           Init
 * 2022-1-20      JasonHu           add map & unmap
 */

#ifndef __ARCH_MMU__
#define __ARCH_MMU__

#include <xbook.h>

/* page table entry (PTE) fields */
#define PTE_P     0x001 // Present
#define PTE_R     0x000 // Read
#define PTE_W     0x002 // Write
#define PTE_X     0x000 // Execute
#define PTE_U     0x004 // User
#define PTE_PWT   0x008 // Write-through
#define PTE_S     0x000 // System
#define PTE_A     0x020 // Accessed
#define PTE_D     0x040 // Dirty

#define NX_PAGE_ATTR_READ     (PTE_R)
#define NX_PAGE_ATTR_WRITE    (PTE_W)
#define NX_PAGE_ATTR_EXEC     (PTE_X)

#define NX_PAGE_ATTR_RWX      (PTE_X | PTE_W | PTE_R)

#define NX_PAGE_ATTR_KERNEL   (PTE_P | NX_PAGE_ATTR_RWX | PTE_S)
#define NX_PAGE_ATTR_USER     (PTE_P | NX_PAGE_ATTR_RWX | PTE_U)

#endif  /* __ARCH_MMU__ */
