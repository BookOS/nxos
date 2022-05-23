/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Platfrom main 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-04     JasonHu           Init
 */

#include <nxos.h>
#define NX_LOG_NAME "Hal Main"
#include <base/log.h>

#ifdef CONFIG_NX_ENABLE_PLATFORM_MAIN
NX_INTERFACE void NX_HalPlatformMain(void)
{
    NX_LOG_I("QEMU platform main running...\n");
}
#endif /* CONFIG_NX_ENABLE_PLATFORM_MAIN */
