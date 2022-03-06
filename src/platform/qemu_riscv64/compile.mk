##
# Copyright (c) 2018-2022, NXOS Development Team
# SPDX-License-Identifier: Apache-2.0
# 
# Contains: Makefile for QEMU Riscv64 Platform
# 
# Change Logs:
# Date           Author            Notes
# 2021-10-1      JasonHu           Init
##

# modify compiler here
ifeq ($(HOSTOS), windows)
CROSS_COMPILE	:= riscv-none-embed-
else
CROSS_COMPILE	:= riscv64-unknown-elf-
endif
