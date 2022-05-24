/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: limits for nxos
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-3-21      JasonHu           Init
 */

#ifndef __UTILS_LIMITS__
#define __UTILS_LIMITS__

#include <nxos.h>

/* Minimum and maximum values a 'signed long int' can hold */
#ifdef CONFIG_NX_CPU_64BIT
#define NX_LONG_MAX		0x7fffffffffffffffL
#else
#define NX_LONG_MAX		0x7fffffffL
#endif

#define NX_LONG_MIN		(-NX_LONG_MAX - 1)

/* Maximum value an 'unsigned long int' can hold (Minimum is 0) */
#define NX_ULONG_MAX    (2UL * NX_LONG_MAX + 1)

#endif  /* __UTILS_LIMITS__ */
