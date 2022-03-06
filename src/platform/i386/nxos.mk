##
# Copyright (c) 2018-2022, NXOS Development Team
# SPDX-License-Identifier: Apache-2.0
# 
# Contains: Makefile for PC32 Platform
# 
# Change Logs:
# Date           Author            Notes
# 2021-9-20      JasonHu           Init
##

#
# Override default variables.
#
CFLAGS		+= -fno-builtin -fno-stack-protector
MCFLAGS		+= -march=i386
LDFLAGS 	+= -no-pie -nostartfile -n 

CC			:= $(CROSS_COMPILE)gcc -m32
AS			:= $(CROSS_COMPILE)gcc -m32
LD			:= $(CROSS_COMPILE)ld -m elf_i386
