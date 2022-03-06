/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Multiboot2 start file
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-9-17      JasonHu           Init
 */

#define __ASSEMBLY__
#include <multiboot2.h>

# the NX_USize of bootStack is 16KB
#define BOOT_STACK_SIZE 0x4000

.code32

.section .text.start
.globl Start, _start
.extern NX_Main
.extern NX_HalBootSetting

Start:
_start:
    jmp MultibootEntry

.align 8
MultibootHeader:
    .long MULTIBOOT2_HEADER_MAGIC                  # magic number (multiboot 2)
    .long MULTIBOOT_ARCHITECTURE_I386              # architecture 0 (protected mode i386)
    .long MultibootHeaderEnd - MultibootHeader  # header length
    # checksum
    .long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (MultibootHeaderEnd - MultibootHeader))
    # insert optional multiboot tags here

    # required end tag
    .align 8
    .short MULTIBOOT_HEADER_TAG_END # type
    .short 0                        # flags
    .long  8                        # NX_USize
MultibootHeaderEnd:
MultibootEntry:
    # initialize the bootStack pointer
    movl $(bootStack + BOOT_STACK_SIZE), %esp

    # reset EFLAGS
    pushl $0
    popf
    
    # push the pointer to the Multiboot information structure
    pushl %ebx
    # push the magic value
    pushl %eax
    
    call NX_HalBootSetting
    cmpl $-1, %eax
    je BootSetupFail

    add $8, %esp

    # call os main
    call NX_Main
    jmp MultibootHlt

BootSetupFail:
    # print "Error!" in protected mode
    movl $0xcf72cf45, 0xb8000
    movl $0xcf6fcf72, 0xb8004
    movl $0xcf21cf72, 0xb8008

MultibootHlt:
    hlt
    jmp MultibootHlt

    .comm bootStack, BOOT_STACK_SIZE
