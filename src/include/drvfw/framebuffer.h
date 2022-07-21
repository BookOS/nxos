/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: framebuffer driver framework
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-06-03     JasonHu           Init
 */

#ifndef __DRVFW_FRAMEBUFFER_H__
#define __DRVFW_FRAMEBUFFER_H__

#include <nxos.h>

#define NX_FRAMEBUFFER_CMD_GETINFO  1

typedef struct NX_FramebufferInfo {
    NX_U8 bitsPerPixel;           /* bits per pixel */
    NX_U16 bytesPerScanLine;      /* bytes per scan line */
    NX_U16 xResolution;           /* horizontal resolution in pixels or characters */
    NX_U16 yResolution;           /* vertical resolution in pixels or characters */
    NX_U32 phyBasePtr;            /* physical address for flat memory frame buffer */
} NX_FramebufferInfo;

#endif  /* __DRVFW_FRAMEBUFFER_H__ */
