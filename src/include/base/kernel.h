/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Kernel header
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-19     JasonHu           Add kernel
 */

#ifndef __XBOOK_KERNEL__
#define __XBOOK_KERNEL__

#include <nx_configure.h>

#ifndef CONFIG_NX_KVADDR_OFFSET
#error "Please config NX_KVADDR_OFFSET"
#else
#define NX_KVADDR_OFFSET CONFIG_NX_KVADDR_OFFSET
#endif

#ifndef CONFIG_NX_MULTI_CORES_NR
#error "Please config NX_MULTI_CORES_NR"
#else
#define NX_MULTI_CORES_NR CONFIG_NX_MULTI_CORES_NR
#endif

#endif  /* __XBOOK_KERNEL__ */
