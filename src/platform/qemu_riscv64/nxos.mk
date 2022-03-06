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

#
# Override default variables.
#
CFLAGS		+= -fno-stack-protector -mno-relax
ASFLAGS		+= -ffunction-sections -fdata-sections -ffreestanding 
MCFLAGS		+= -march=rv64imafdc -mabi=lp64d -mcmodel=medany
LDFLAGS 	+= -no-pie -nostartfile -n 
