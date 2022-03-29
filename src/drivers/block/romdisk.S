/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: romdisk from cpio binary
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-3-21      JasonHu           Init
 */

#define __ASSEMBLY__
#include <xbook.h>

#ifdef CONFIG_NX_DRIVER_ROMDISK

.section .romdisk, "a"
.incbin CONFIG_NX_DRIVER_ROMDISK_HOSTOS_PATH /* path from current dir */

#endif
