/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: riscv define
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-3      JasonHu           Init
 */

#ifndef __PLATFORM_RISCV__
#define __PLATFORM_RISCV__

#include <nx_configure.h>

#ifdef CONFIG_NX_CPU_64BITS
#define STORE                   sd
#define LOAD                    ld
#define REGBYTES                8
#else
#error "not support 32bit!"
#endif

/* local interrupt controller, which contains the timer. */
#define RISCV_CLINT_PADDR       0x02000000UL
/* Platform level interrupt controller */
#define RISCV_PLIC_PADDR        0x0c000000UL

#endif  /* __PLATFORM_RISCV__ */
