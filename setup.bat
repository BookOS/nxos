::
:: Copyright (c) 2018-2022, NXOS Development Team
:: SPDX-License-Identifier: Apache-2.0
:: 
:: Contains: windows bat scripts for nxos environment
:: 
:: Change Logs:
:: Date           Author            Notes
:: 2022-1-24      JasonHu           Init
:: 2022-05-24     JasonHu           remove arch name arg
::

:: usage:
:: setup.bat [arch-platform]
:: example: setup.bat               # i386
:: example: setup.bat qemu_riscv64  # qemu_riscv64

@echo off

set def_target=%1

if "%def_target%" == "" (
    :: set default targe as i386
    set def_target=i386
)

echo Set environment for NXOS.
set NXOS_SRC_DIR=%cd%/src

echo [SRC DIR  ] %NXOS_SRC_DIR%
echo [PLAFORM  ] %def_target%
cp configs/platform-%def_target%.mk platform.mk

@echo on
