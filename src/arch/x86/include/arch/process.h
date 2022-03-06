/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Arch process
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-7       JasonHu           Init
 */

#ifndef __ARCH_PROCESS___
#define __ARCH_PROCESS___

#define NX_USER_STACK_TOP     0xFFFFF000UL          /* stack end */
#define NX_USER_STACK_VADDR   0xC0000000UL          /* stack start */
#define NX_USER_MAP_TOP       NX_USER_STACK_VADDR   /* map end */
#define NX_USER_MAP_VADDR     0xA0000000UL          /* map start */
#define NX_USER_HEAP_TOP      NX_USER_MAP_VADDR     /* heap end */
#define NX_USER_HEAP_VADDR    0x80000000UL          /* heap start */
#define NX_USER_IMAGE_TOP     NX_USER_HEAP_VADDR    /* code & data */
#define NX_USER_IMAGE_VADDR   0x40000000UL          /* code & data */

#define NX_USER_SPACE_VADDR   NX_USER_IMAGE_VADDR
#define NX_USER_SPACE_TOP     NX_USER_STACK_TOP
#define NX_USER_SPACE_SIZE    (NX_USER_SPACE_TOP - NX_USER_SPACE_VADDR)

#endif /* __ARCH_PROCESS___ */
