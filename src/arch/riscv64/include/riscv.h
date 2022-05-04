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

#if defined(CONFIG_NX_PLATFORM_K210) || \
    defined(CONFIG_NX_PLATFORM_RISCV64_QEMU) || \
    defined(CONFIG_NX_PLATFORM_HIFIVE_UNMACHED)

/* local interrupt controller, which contains the timer. */
#define RISCV_CLINT_PADDR       0x02000000UL
/* Platform level interrupt controller */
#define RISCV_PLIC_PADDR        0x0c000000UL

#elif defined(CONFIG_NX_PLATFORM_D1)

#define RISCV_CLINT_PADDR       0x04000000UL
#define RISCV_PLIC_PADDR        0x10000000UL

#else

#error "pleae check your CLINT and PLIC addr"

#endif

#if defined(CONFIG_NX_PLATFORM_HIFIVE_UNMACHED)
#define NX_VALID_HARTID_OFFSET 1 /* hart0 is u540, don't use it! */
#else
#define NX_VALID_HARTID_OFFSET 0
#endif

#endif  /* __PLATFORM_RISCV__ */
