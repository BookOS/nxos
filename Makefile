##
# Copyright (c) 2018-2022, NXOS Development Team
# SPDX-License-Identifier: Apache-2.0
# 
# Contains: Makefile for NXOS
# 
# Change Logs:
# Date           Author            Notes
# 2021-9-20      JasonHu           Init
##

#
# Get platform config
#
sinclude platform.mk

#
# Tools name
#

#
# Use build dir for target
#
USE_BUILD_DIR	:=y
BUILD_DIR :=build

#
# Set compile and platform
#
CROSS_COMPILE	?=
ARCH_PLATFORM	?=

#
# Target file name
#
NXOS_NAME ?= NXOS
export NXOS_NAME

#
# Use kconfiglib
#
USE_KCONFIGLIB	:=y
KCONFIGLIB_DIR	:= scripts/kconfiglib

#
# Enable GDB debug
#
G	?=n

#
# Get platform information about ARCH and PLATFORM from ARCH_PLATFORM variable.
#
ifeq ($(words $(subst -, , $(ARCH_PLATFORM))), 2)
ARCH			:= $(word 1, $(subst -, , $(ARCH_PLATFORM)))
PLATFORM		:= $(word 2, $(subst -, , $(ARCH_PLATFORM)))
else
# you can set default platfrom here
ARCH			:= x86
PLATFORM		:= i386
endif

#
# System environment variable.
#
ifeq ($(OS), Windows_NT)
	HOSTOS		:= windows
else
	ifneq (,$(findstring Linux, $(shell uname -a)))
		HOSTOS	:= linux
	endif
endif

#
# Override default variables.
#
sinclude src/platform/$(PLATFORM)/compile.mk

#
# Export global values
#
export CROSS_COMPILE
export PLATFORM
export ARCH
export HOSTOS
export USE_BUILD_DIR
export G

#
# Kconfig path
#
CONFIG_OUT_FILE = ./src/include/nx_configure.h
CONFIG_OUT_FILE_PLATFORM = ./src/platform/$(PLATFORM)/include/nx_configure.h
CONFIG_IN_FILE = .config

#
# Cmds
#
.PHONY: all clean run prepare menuconfig defconfig saveconfig gdb cleanAll

#
# Compile only
#
all:
ifeq ($(USE_BUILD_DIR), y)
	@$(MAKE) -s -C src O=$(BUILD_DIR)
else
	@$(MAKE) -s -C src
endif

cleanAll: clean
	@-rm -f .config
	@-rm -f .config.old
	@-rm -f ./src/platform/Kconfig
	@-rm -f ./src/include/nx_configure.h

#
# Clean all targets
#
clean:
ifeq ($(USE_BUILD_DIR), y)
	@$(MAKE) -s -C src clean O=$(BUILD_DIR)
else
	@$(MAKE) -s -C src clean
endif
	@$(MAKE) -s -C src/platform/$(PLATFORM) -f cmd.mk clean

#
# Run OS
#
run: all 
	@$(MAKE) -s -C src/platform/$(PLATFORM) -f cmd.mk run

#
# Prepare platform tools
#
prepare: 
	@$(MAKE) -s -C src/platform/$(PLATFORM) -f cmd.mk prepare

#
# GDB command
#
gdb:
	@$(MAKE) -s -C src/platform/$(PLATFORM) -f cmd.mk gdb

#
# dump kernel
#
dump:
	@$(MAKE) -s -C src/platform/$(PLATFORM) -f cmd.mk dump

#
# menuconfig
#
menuconfig:
ifeq ($(USE_KCONFIGLIB), y)
	@python $(KCONFIGLIB_DIR)/menuconfig.py Kconfig
	@python $(KCONFIGLIB_DIR)/genconfig.py --header-path=$(CONFIG_OUT_FILE).tmp
	@echo "#ifndef __NX_CONFIG__" > $(CONFIG_OUT_FILE)
	@echo "#define __NX_CONFIG__" >> $(CONFIG_OUT_FILE)
	@cat $(CONFIG_OUT_FILE).tmp >> $(CONFIG_OUT_FILE)
	@echo "#endif" >> $(CONFIG_OUT_FILE)
	@rm -f $(CONFIG_OUT_FILE).tmp
	@echo genconfig to $(CONFIG_OUT_FILE) .
endif

defconfig:
	@-rm -f .config
	@-rm -f .config.old
	@-cp src/platform/$(PLATFORM)/defconfig ./.config
	@-cp src/platform/$(PLATFORM)/Kconfig ./src/platform/Kconfig
	@-cp $(CONFIG_OUT_FILE_PLATFORM) $(CONFIG_OUT_FILE)
	@echo update Kconfig from platform $(ARCH)-$(PLATFORM) .

saveconfig:
	@-cp ./.config src/platform/$(PLATFORM)/defconfig
	@-cp ./src/platform/Kconfig ./src/platform/$(PLATFORM)/Kconfig 
	@-cp $(CONFIG_OUT_FILE) $(CONFIG_OUT_FILE_PLATFORM)
	@echo save Kconfig to platform $(ARCH)-$(PLATFORM) .
