/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Link script for PC32 Platfrom
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-9-12      JasonHu           Init
 */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(Start)

BASE_ADDRESS = 0x00100000; /* GRUB2 need kernel here */

SECTIONS {
    /* Load the kernel at this address: "." means the current address */
    
    . = BASE_ADDRESS;

    /* code segment */
    . = ALIGN(4);
    __NX_TextStart = .;
    .text : 
    {
        *(.text.start)
        *(.text)
        *(.init.text)
		*(.exit.text)
        
        . = ALIGN(4);
        PROVIDE(__NX_InitCallStart = .);
        KEEP(*(.initCall0.text))
        KEEP(*(.initCall1.text))
        KEEP(*(.initCall2.text))
        KEEP(*(.initCall9.text))
        PROVIDE(__NX_InitCallEnd = .);

        . = ALIGN(4);
        PROVIDE(__NX_ExitCallStart = .);
        KEEP(*(.exitcall0.text))
        KEEP(*(.exitcall1.text))
        KEEP(*(.exitcall2.text))
        PROVIDE(__NX_ExitCallEnd = .);
    
        /* section information for utest */
        . = ALIGN(4);
        PROVIDE(__NX_UTestCaseTableStart = .);
        KEEP(*(UTestCaseTable))
        PROVIDE(__NX_UTestCaseTableEnd = .);
        
        /* section information for integration */
        . = ALIGN(4);
        PROVIDE(__NX_IntegrationTableStart = .);
        KEEP(*(IntegrationTable))
        PROVIDE(__NX_IntegrationTableEnd = .);
    }
    __NX_TextEnd = .;


    /* data segment */
    . = ALIGN(4);
    __NX_DataStart = .;
    .data :
    {
        *(.data)
        *(.data.*)
        *(.rodata)
        *(.rodata.*)
    }
    __NX_DataEnd = .;
    
    /* bss segement */
    
    . = ALIGN(4);
    __NX_BssStart = .;
    .bss       :
    {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
    . = ALIGN(4);
    __NX_BssEnd = .;

}
