/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Link script for allwinner-d1 Platform
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-17      JasonHu           Init
 */

OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_Start)

MEMORY
{
    /* opensbi jump to 0x40200000 */
    SRAM : ORIGIN = 0x40200000, LENGTH = 0x1E00000
}

SECTIONS
{
    /* code segment */
    .text : 
    {
        __NX_TextStart = .;
        *(.text.start)
        *(.text)
        *(.init.text)
		*(.exit.text)

        *(.rodata)
        *(.rodata.*)

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
        
        __NX_TextEnd = .;
    } > SRAM


    /* data segment */
    . = ALIGN(8);

    .data :
    {
        __NX_DataStart = .;
        *(.data)
        *(.data.*)

        . = ALIGN(8);
        PROVIDE( __global_pointer$ = . + 0x800 );

        *(.sdata)
        *(.sdata.*)
    } > SRAM

	.romdisk ALIGN(8) :
	{
		PROVIDE(__NX_RomdiskStart = .);
		KEEP(*(.romdisk))
		PROVIDE(__NX_RomdiskEnd = .);
	    __NX_DataEnd = .;
    } > SRAM
    . = ALIGN(8);
        
    /* bss segement */

    .sbss : 
    {
        __NX_BssStart = .;
        *(.sbss)
        *(.sbss.*)
        *(.dynsbss)
        *(.scommon)
    } > SRAM

    .bss :
    {
        *(.bss)
        *(.bss.*)
        *(.dynbss)
        *(COMMON)
        . = ALIGN(8);
        __NX_BssEnd = .;
    } > SRAM
}