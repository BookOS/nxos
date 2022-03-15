/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Link script for Riscv64 k210 Platform
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-25      JasonHu           Init
 */

OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_Start)

/* rustsbi defined here */
BASE_ADDRESS = 0x80020000;

SECTIONS
{
    /* Load the OS at this address: "." means the current address */
    . = BASE_ADDRESS;

    /* code segment */
    . = ALIGN(8);
    __NX_TextStart = .;
    .text : 
    {
        *(.text.start)
        *(.text)
        *(.init.text)
		*(.exit.text)
        
        . = ALIGN(8);
        PROVIDE(__NX_InitCallStart = .);
        KEEP(*(.initCall0.text))
        KEEP(*(.initCall1.text))
        KEEP(*(.initCall2.text))
        KEEP(*(.initCall3.text))
        KEEP(*(.initCall4.text))
        KEEP(*(.initCall5.text))
        KEEP(*(.initCall6.text))
        KEEP(*(.initCall7.text))
        KEEP(*(.initCall8.text))
        KEEP(*(.initCall9.text))
        PROVIDE(__NX_InitCallEnd = .);

        . = ALIGN(8);
        PROVIDE(__NX_ExitCallStart = .);
        KEEP(*(.exitcall0.text))
        KEEP(*(.exitcall1.text))
        KEEP(*(.exitcall2.text))
        KEEP(*(.exitcall3.text))
        KEEP(*(.exitcall4.text))
        KEEP(*(.exitcall5.text))
        KEEP(*(.exitcall6.text))
        KEEP(*(.exitcall7.text))
        KEEP(*(.exitcall8.text))
        KEEP(*(.exitcall9.text))
        PROVIDE(__NX_ExitCallEnd = .);
    
        /* section information for utest */
        . = ALIGN(8);
        PROVIDE(__NX_UTestCaseTableStart = .);
        KEEP(*(UTestCaseTable))
        PROVIDE(__NX_UTestCaseTableEnd = .);
        
        /* section information for integration */
        . = ALIGN(8);
        PROVIDE(__NX_IntegrationTableStart = .);
        KEEP(*(IntegrationTable))
        PROVIDE(__NX_IntegrationTableEnd = .);
    }
    __NX_TextEnd = .;


    /* data segment */
    . = ALIGN(8);
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
    
    . = ALIGN(8);
    __NX_BssStart = .;
    .bss       :
    {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
    . = ALIGN(8);
    __NX_BssEnd = .;

}