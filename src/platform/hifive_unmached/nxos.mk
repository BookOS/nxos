##
# Copyright (c) 2018-2022, NXOS Development Team
# SPDX-License-Identifier: Apache-2.0
# 
# Contains: Makefile for hifive unmached Platform
# 
# Change Logs:
# Date           Author            Notes
# 2022-4-17      JasonHu           Init
##

#
# Override default variables.
#

CFLAGS		+= -fvar-tracking
ASFLAGS		+= -ffunction-sections -fdata-sections -ffreestanding 
MCFLAGS		+= -march=rv64imafdc -mabi=lp64d -mcmodel=medany
LDFLAGS 	+= -no-pie -nostartfile -n 
