##
# Copyright (c) 2018-2022, NXOS Development Team
# SPDX-License-Identifier: Apache-2.0
# 
# Contains: Makefile for run PC32 Platform
# 
# Change Logs:
# Date           Author            Notes
# 2021-9-20      JasonHu           Init
##

#
# Config graphic window ? (y/n)
# 
QEMU_WINDOW ?= n

#
# Tools for PC32
# 
TOOL_DIR 	:= tools/
GRUB_DIR	:= $(TOOL_DIR)grub-2.04
BIOS_FW_DIR	:= $(TOOL_DIR)bios_fw
RM			:= rm

#
# Tools name
# 
MAKE		:= make
QEMU 		:= qemu-system-i386
DEBUGER		:= $(CROSS_COMPILE)gdb
DUMP		:= $(CROSS_COMPILE)objdump

#
# Qemu args configure
# 
QEMU_ARGS 	:=  -m 512m \
				-name "NXOS - Plartform PC32 - Arch x86" \
				-rtc base=localtime \
				-boot d \
				-cdrom $(NXOS_NAME).iso \

ifeq ($(QEMU_WINDOW),y)
	QEMU_ARGS += -serial stdio
else
	QEMU_ARGS += -nographic
endif

ifeq ($(G),y)
	QEMU_ARGS += -S -gdb tcp::10001,ipv4
endif

#
# Args for make
#
.PHONY: run clean

#
# Run OS in QEMU
# 
run:
	@$(MAKE) -s -C $(GRUB_DIR) KERNEL=$(NXOS_NAME).elf OS_NAME=$(NXOS_NAME)
	$(QEMU) $(QEMU_ARGS)

# 
# Clear target file
# 
clean:
	@$(MAKE) -s -C $(GRUB_DIR) clean
	-$(RM) $(NXOS_NAME).elf

# 
# prepare tools
# 
prepare:
	-$(RM) -rf $(TOOL_DIR)
	git clone https://gitee.com/BookOS/nxos-platform-pc32-tools $(TOOL_DIR)
	echo "parpare done."

# 
# gdb debug
# 
gdb:
	@echo gdb load file from $(ARCH)/$(PLATFORM)/$(NXOS_NAME).elf
	$(DEBUGER) $(NXOS_NAME).elf -x connect.gdb
	
#
# dump kernel
#
dump:
	@echo gdb load file from $(ARCH)/$(PLATFORM)/$(NXOS_NAME).elf
	$(DUMP) -D -S $(NXOS_NAME).elf > $(NXOS_NAME).dump.S
	