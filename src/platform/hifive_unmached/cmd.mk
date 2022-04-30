##
# Copyright (c) 2018-2022, NXOS Development Team
# SPDX-License-Identifier: Apache-2.0
# 
# Contains: Makefile for run hifive unmached Platform
# 
# Change Logs:
# Date           Author            Notes
# 2022-05-02     JasonHu           Init
##

RM			:= rm
CP			:= cp
LYNX		:= lynx
SSH_CP		:= scp
DUMP		:= $(CROSS_COMPILE)objdump
REMOTE_IP	?= 183.173.21.114
SSH_PORT	?= 8122
HTTP_PORT	?= 8182

ACTION		?= reboot # poweron, powerff, reboot
MACHINE_ID	?= 2 # 1, 2

#
# Args for make
#
.PHONY: run clean

#
# upload nxos.elf to remote hifive_unmached
# 
run:
	echo "hifive_unmached run..."
	scp -P $(SSH_PORT) $(NXOS_NAME).elf ubuntu@$(REMOTE_IP):/srv/tftp/nxos.elf
	$(LYNX) http://$(REMOTE_IP):$(HTTP_PORT)/$(ACTION)$(MACHINE_ID)
	echo "start hifive_unmached done."

# 
# Clear target file
# 
clean:
	-$(RM) $(NXOS_NAME).elf
	-$(RM) $(NXOS_NAME).dump.S

# 
# prepare tools
# 
prepare:
	echo "parpare done."

# 
# gdb debug
# 
gdb:
	echo "gdb not supported!"

#
# dump kernel
#
dump:
	@echo dump kernel $(ARCH)/$(PLATFORM)/$(NXOS_NAME).elf
	$(DUMP) -D -S $(NXOS_NAME).elf > $(NXOS_NAME).dump.S
