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
# Tools
# 
TOOL_DIR 	:= tools
RUSTSBI_DIR	:= $(TOOL_DIR)/SBI
SBI			:= $(TOOL_DIR)/SBI/rustsbi-k210
RM			:= rm
MAKE		:= make
SU			:= sudo
PYTHON		:= python
CP			:= cp
DD			:= dd
DEBUGER		:= $(CROSS_COMPILE)gdb
DUMP		:= $(CROSS_COMPILE)objdump
OC			:= $(CROSS_COMPILE)objcopy

K210_BIN	:= k210.bin
UART 		?= /dev/ttyUSB0
KFLASH 		:= ./tools/kflash.py

#
# Args for make
#
.PHONY: run clean

#
# flush into k210 devboard
# 
run:
	$(OC) $(NXOS_NAME).elf --strip-all -O binary $(NXOS_NAME).bin
	$(OC) $(SBI) --strip-all -O binary $(K210_BIN)
	$(DD) if=$(NXOS_NAME).bin of=$(K210_BIN) bs=128k seek=1
	echo "K210 run..."
ifeq ($(HOSTOS),linux)
	$(SU) chmod 777 $(UART)
endif
	$(PYTHON) $(KFLASH) -p $(UART) -b 1500000 -t $(K210_BIN)

# 
# Clear target file
# 
clean:
	-$(RM) $(NXOS_NAME).elf
	-$(RM) $(K210_BIN)
	-$(RM) $(NXOS_NAME).dump.S
	-$(RM) $(NXOS_NAME).bin

# 
# prepare tools
# 
prepare:
	-$(RM) -rf $(TOOL_DIR)
	git clone https://gitee.com/BookOS/nxos-platform-k210-tools $(TOOL_DIR)
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
	