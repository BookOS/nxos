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

.section .romdisk, "a"
.incbin "../../romdisk.cpio" /* from root dir */
