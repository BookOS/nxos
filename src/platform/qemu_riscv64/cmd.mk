##
# Copyright (c) 2018-2022, NXOS Development Team
# SPDX-License-Identifier: Apache-2.0
# 
# Contains: Makefile for run QEMU Riscv64 Platform
# 
# Change Logs:
# Date           Author            Notes
# 2021-10-1      JasonHu           Init
##

#
# Config graphic window ? (y/n)
# 
QEMU_WINDOW ?= n

#
# Tools
# 
TOOL_DIR 	:= tools
RUSTSBI_DIR	:= $(TOOL_DIR)/SBI
SBI			:= $(TOOL_DIR)/SBI/opensbi-qemu
RM			:= rm

#
# Tools name
# 
MAKE		:= make
QEMU 		:= qemu-system-riscv64
DEBUGER		:= $(CROSS_COMPILE)gdb
DUMP		:= $(CROSS_COMPILE)objdump

#
# Qemu args configure
# 
QEMU_ARGS 	:=  -m 256m \
				-name "NXOS - Plartform QEMU - Arch Riscv64"

# cpus
ifndef CORES
CORES := 1
endif # CORES

QEMU_ARGS	+= 	-machine virt
QEMU_ARGS	+= 	-kernel $(NXOS_NAME).elf
QEMU_ARGS	+= 	-bios $(SBI)
QEMU_ARGS	+= 	-smp $(CORES)

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
	$(QEMU) $(QEMU_ARGS)

# 
# Clear target file
# 
clean:
	-$(RM) $(NXOS_NAME).elf

# 
# prepare tools
# 
prepare:
	-$(RM) -rf $(TOOL_DIR)
	git clone https://gitee.com/BookOS/nxos-platform-qemu-riscv64-tools $(TOOL_DIR)
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
	@echo dump kernel $(ARCH)/$(PLATFORM)/$(NXOS_NAME).elf
	$(DUMP) -D -S $(NXOS_NAME).elf > $(NXOS_NAME).dump.S
	