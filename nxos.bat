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
:: nxos.bat [arch-platform]
:: example: nxos.bat                       # x86-i386
:: example: nxos.bat riscv64-qemu_riscv64  # riscv64-qemu_riscv64%

@echo off

set def_target=%1

if "%def_target%" == "" (
    :: set default targe as x86-i386
    set def_target=x86-i386
)

echo Set environment for NXOS.
set NXOS_NAME=NXOS
set NXOS_ROOT_DIR=%cd%
set NXOS_SRC_DIR=%cd%/src

if "%def_target%" == "x86-i386" (
    set NXOS_EXEC_PATH=%NXOS_ROOT_DIR%\tools\gcc\x86_64-elf-gcc\bin
) else if "%def_target%" == "riscv64-qemu_riscv64" (
    set NXOS_EXEC_PATH=%NXOS_ROOT_DIR%\tools\gcc\riscv-none-embed-gcc\bin
) else if "%def_target%" == "riscv64-k210" (
    set NXOS_EXEC_PATH=%NXOS_ROOT_DIR%\tools\gcc\riscv-none-embed-gcc\bin
) else (
    echo unknown target! %def_target%
    goto end
)

set PATH=%NXOS_EXEC_PATH%;%PATH%;

echo [ROOT DIR ] %NXOS_ROOT_DIR%
echo [SRC DIR  ] %NXOS_SRC_DIR%
echo [EXEC PATH] %NXOS_EXEC_PATH%
cp configs/platform-%def_target%.mk platform.mk
echo [PLAFORM  ] %def_target%

:end
@echo on
