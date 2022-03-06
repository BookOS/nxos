/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Supervisor Binary Interface(SBI)
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-26      JasonHu           Init
 */

#ifndef __RISCV_SBI__
#define __RISCV_SBI__

#include <xbook.h>

#ifdef CONFIG_NX_PLATFROM_K210
#include <rustsbi.h>
#else
#include <opensbi.h>
#endif

#endif  /* __RISCV_SBI__ */
