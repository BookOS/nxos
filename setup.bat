::
:: Copyright (c) 2018-2022, NXOS Development Team
:: SPDX-License-Identifier: Apache-2.0
:: 
:: Contains: windows bat scripts for nxos environment
:: 
:: Change Logs:
:: Date           Author            Notes
:: 2022-1-24      JasonHu           Init
::

:: usage:
:: setup.bat [arch-platform]
:: example: setup.bat                       # x86-i386
:: example: setup.bat riscv64-qemu_riscv64  # riscv64-qemu_riscv64%

@echo off

set def_target=%1

if "%def_target%" == "" (
    :: set default targe as x86-i386
    set def_target=x86-i386
)

echo Set environment for NXOS.
set NXOS_SRC_DIR=%cd%/src

echo [SRC DIR  ] %NXOS_SRC_DIR%
echo [PLAFORM  ] %def_target%
cp configs/platform-%def_target%.mk platform.mk

@echo on
