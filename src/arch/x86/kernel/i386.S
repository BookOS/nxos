/*
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021/10/1      JasonHu      The first version
 */

#define __ASSEMBLY__
#include <segment.h>

.code32
.text

.global CPU_LoadIDT
CPU_LoadIDT:
    movl 4(%esp), %eax
    movw %ax, 6(%esp)
    lidt 6(%esp)
    ret

.global CPU_LoadGDT
CPU_LoadGDT:
    movl 4(%esp), %eax
    movw %ax, 6(%esp)
    lgdt 6(%esp)
    
    # flush segment registers
    movw $KERNEL_DATA_SEL, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    xor %eax, %eax
    movw %ax, %fs
    movw %ax, %gs
    ljmp $KERNEL_CODE_SEL, $.1
.1:
    ret
