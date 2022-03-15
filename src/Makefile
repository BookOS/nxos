##
# Copyright (c) 2018-2022, NXOS Development Team
# SPDX-License-Identifier: Apache-2.0
# 
# Contains: Makefile for Source code
# 
# Change Logs:
# Date           Author            Notes
# 2021-9-20      JasonHu           Init
##

#
# include xbuild scripts
#
sinclude ../scripts/xbuild/env.mk

#
# Load default variables.
#
ifeq ($(G),y)
ASFLAGS			:= -g -ggdb -O0 -Wall
CFLAGS			:= -g -ggdb -O0 -Wall
else
ASFLAGS			:= -Wall -O3
CFLAGS			:= -Wall -O3
endif
LDFLAGS			:= -T $(srctree)/platform/$(PLATFORM)/nxos.ld -nostdlib -static
MCFLAGS			:=

LIBDIRS			:=
LIBS 			:=
INCDIRS			:=
SRCDIRS			:=

CP				:=cp

#
# Override default variables.
#
sinclude $(srctree)/platform/$(PLATFORM)/nxos.mk

#
# Modify default variables.
#
ifeq ($(strip $(ARCH)), arm32)
DEFINES		+=	-D__ARM__
endif
ifeq ($(strip $(ARCH)), arm64)
DEFINES		+=	-D__ARM64__
endif
ifeq ($(strip $(ARCH)), riscv64)
DEFINES		+=	-D__RISCV64__
endif
ifeq ($(strip $(ARCH)), riscv32)
DEFINES		+=	-D__RISCV32__
endif
ifeq ($(strip $(ARCH)), amd64)
DEFINES		+=	-D__AMD64__
endif
ifeq ($(strip $(ARCH)), x86)
DEFINES		+=	-D__I386__
endif

RELEASE 	?=n

# debug info
ifeq ($(RELEASE),n)
ASFLAGS		+=	-g
CFLAGS		+=	-g
endif

ASFLAGS		+=	-ffunction-sections -fdata-sections -ffreestanding -std=gnu99 $(DEFINES)
CFLAGS		+=	-ffunction-sections -fdata-sections -ffreestanding -std=gnu99 $(DEFINES)

#
# Add necessary directory for INCDIRS and SRCDIRS.
#
INCDIRS		+=	include
INCDIRS		+=	./
INCDIRS		+=	arch/$(ARCH)/include
INCDIRS		+=	platform/$(PLATFORM)/include

# Srouce codes
SRCDIRS		+=	arch/$(ARCH)/
SRCDIRS		+=	platform/$(PLATFORM)/
SRCDIRS		+=	init/
SRCDIRS		+=	mods/
SRCDIRS		+=	utils/
SRCDIRS		+=	mm/
SRCDIRS		+=	tests/
SRCDIRS		+=	sched/
SRCDIRS		+=	io/
SRCDIRS		+=	process/
SRCDIRS		+=	kernel/

#
# XBuild variables
#
X_ASFLAGS	:=	$(MCFLAGS) $(ASFLAGS)
X_CFLAGS	:=	$(MCFLAGS) $(CFLAGS)
X_LDFLAGS	:=	$(LDFLAGS)
X_ODFLAGS	:=	$(ODFLAGS)
X_INCDIRS	:=	$(INCDIRS)
SRC			:=	$(SRCDIRS)
NAME		:= platform/$(PLATFORM)/$(NXOS_NAME).elf

#
# Custom cmds for xbuild
# Link OS and copy into platform dir
#

ifeq ($(USE_BUILD_DIR), y)

define CUSTOM_TARGET_CMD
echo [KERNEL] $@; \
$(LD) $(X_LDFLAGS) -o $@ $(X_OBJS); \
$(CP) $@ $(srctree)/platform/$(PLATFORM); \
$(CP) $@ ./
endef # CUSTOM_TARGET_CMD

else

define CUSTOM_TARGET_CMD
echo [KERNEL] $@; \
$(LD) $(X_LDFLAGS) -o $@ $(X_OBJS); \
$(CP) $@ ./
endef # CUSTOM_TARGET_CMD

endif # USE_BUILD_DIR
