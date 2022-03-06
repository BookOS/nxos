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
##

# usage:
# source nxos.sh [arch-platform]
# example: source nxos.sh                       # x86-i386
# example: source nxos.sh riscv64-qemu_riscv64  # riscv64-qemu_riscv64

# supported targe list
supported_target="x86-i386 riscv64-qemu_riscv64 riscv64-k210"

def_target="unknown" 

# find target in target list
if [ -z $1 ]
then
    def_target="x86-i386" # default target is x86-i386
else
    for target in $supported_target
    do
        if [ $target = $1 ]
        then
            def_target=$target
            break
        fi
    done
fi

echo "Set environment for NXOS."
export NXOS_NAME="NXOS"
export NXOS_ROOT_DIR=$(pwd)
export NXOS_SRC_DIR=$NXOS_ROOT_DIR/src

# set env
case $def_target in 
    "x86-i386")
        export NXOS_EXEC_PATH=$NXOS_ROOT_DIR/tools/gcc/x86_64-elf-gcc/bin
        ;;
    "riscv64-qemu_riscv64")
        export NXOS_EXEC_PATH=$NXOS_ROOT_DIR/tools/gcc/riscv64-unknown-elf-gcc/bin
        ;;
    "riscv64-k210")
        export NXOS_EXEC_PATH=$NXOS_ROOT_DIR/tools/gcc/riscv64-unknown-elf-gcc/bin
        ;;
    *)  echo "unknown target! " $def_target
        return 1
esac

# export NXOS_EXEC_PATH
export PATH=$NXOS_EXEC_PATH:$PATH

echo "[ROOT DIR ]" $NXOS_ROOT_DIR
echo "[SRC DIR  ]" $NXOS_SRC_DIR
echo "[EXEC PATH]" $NXOS_EXEC_PATH
cp configs/platform-$def_target.mk platform.mk
echo "[PLAFORM  ]" $def_target