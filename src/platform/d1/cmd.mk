##
# Copyright (c) 2018-2022, NXOS Development Team
# SPDX-License-Identifier: Apache-2.0
# 
# Contains: Makefile for run d1 Platform
# 
# Change Logs:
# Date           Author            Notes
# 2022-4-17      JasonHu           Init
##

#
# Tools
# 
TOOL_DIR 	:= ./tools
SBI			:= $(TOOL_DIR)/SBI/opensbi-d1.bin
XFEL		:= $(TOOL_DIR)/xfel/xfel.exe
RM			:= rm
MAKE		:= make
SU			:= sudo
PYTHON		:= python
CP			:= cp
DD			:= dd
DEBUGER		:= $(CROSS_COMPILE)gdb
DUMP		:= $(CROSS_COMPILE)objdump
OC			:= $(CROSS_COMPILE)objcopy

#
# Args for make
#
.PHONY: run clean

#
# flush into d1 devboard
# 
run:
	$(OC) $(NXOS_NAME).elf --strip-all -O binary $(NXOS_NAME).bin
	echo "allwinner-d1 run..."
	$(XFEL) version
	$(XFEL) ddr d1
	$(XFEL) write 0x40000000 $(SBI)
	$(XFEL) write 0x40200000 $(NXOS_NAME).bin
	$(XFEL) exec 0x40000000 
	echo "start d1 done."
	
# run d1 with xfel

# 
# Clear target file
# 
clean:
	-$(RM) $(NXOS_NAME).elf
	-$(RM) $(NXOS_NAME).dump.S
	-$(RM) $(NXOS_NAME).bin

# 
# prepare tools
# 
prepare:
	-$(RM) -rf $(TOOL_DIR)
	git clone https://gitee.com/BookOS/nxos-platform-d1-tools $(TOOL_DIR)
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
	