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

# modify compiler here
ifeq ($(HOSTOS), windows)
CROSS_COMPILE	:= riscv-none-embed-
else
CROSS_COMPILE	:= riscv64-linux-gnu-
endif
