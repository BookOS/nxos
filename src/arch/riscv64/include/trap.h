/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: riscv trap
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-3      JasonHu           Init
 */

#ifndef __RISCV_TRAP__
#define __RISCV_TRAP__

#include <xbook.h>
#include <interrupt.h>

void CPU_InitTrap(NX_UArch coreId);

void TrapFrameDump(NX_HalTrapFrame *frame);

#endif  /* __RISCV_TRAP__ */
