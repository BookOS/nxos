/**
 * Copyright (c) 2018-2022, BookOS Development Team
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

#define ARCH_USER_SPACE_TOP     0x400000000UL           /* user space top */
#define ARCH_USER_STACK_TOP     0x3FFFFF000UL           /* stack end */
#define ARCH_USER_STACK_VADDR   0x380000000UL           /* stack start */
#define ARCH_USER_MAP_TOP       ARCH_USER_STACK_VADDR   /* map end */
#define ARCH_USER_MAP_VADDR     0x300000000UL           /* map start */
#define ARCH_USER_HEAP_TOP      ARCH_USER_MAP_VADDR     /* heap end */
#define ARCH_USER_HEAP_VADDR    0x200000000UL           /* heap start */
#define ARCH_USER_IMAGE_TOP     ARCH_USER_HEAP_VADDR    /* code & data */
#define ARCH_USER_IMAGE_VADDR   0x100000000UL           /* code & data */

#define ARCH_USER_SPACE_VADDR   ARCH_USER_IMAGE_VADDR
#define ARCH_USER_SPACE_SIZE    (ARCH_USER_SPACE_TOP - ARCH_USER_SPACE_VADDR)

#endif /* __ARCH_PROCESS___ */
