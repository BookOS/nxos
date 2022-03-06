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

ifeq ($(HOSTOS), windows)
CROSS_COMPILE	:= x86_64-elf-
else
CROSS_COMPILE	:= 
endif
