#!/bin/bash

##
# Copyright (c) 2018-2022, NXOS Development Team
# SPDX-License-Identifier: Apache-2.0
# 
# Contains: shell scripts for nxos environment
# 
# Change Logs:
# Date           Author            Notes
# 2022-1-24      JasonHu           Init
# 2022-05-24     JasonHu           remove arch name arg
##

# usage:
# source setup.sh [arch-platform]
# example: source setup.sh                       # i386
# example: source setup.sh riscv64-qemu_riscv64  # riscv64-qemu_riscv64

if [ -z $1 ]
then
    def_target="i386" # default target is i386
else
    def_target=$1
fi

echo "Set environment for NXOS."
export NXOS_SRC_DIR=$(pwd)/src

echo "[SRC DIR  ]" $NXOS_SRC_DIR
echo "[PLAFORM  ]" $def_target
cp configs/platform-$def_target.mk platform.mk
