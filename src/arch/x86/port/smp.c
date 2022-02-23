/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: HAL Multi Core support
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-9      JasonHu           Init
 */

#include <sched/smp.h>
#define NX_LOG_NAME "Multi Core"
#include <utils/log.h>

NX_UArch HAL_CoreGetIndex(void)
{
    return 0;
}

NX_Error HAL_CoreBootApp(NX_UArch bootCoreId)
{
    NX_LOG_I("boot core is:%d", bootCoreId);
    return NX_ENORES;
}

NX_Error HAL_CoreEnterApp(NX_UArch appCoreId)
{
    return NX_ENORES;
}

NX_INTERFACE struct NX_SMP_Ops NX_SMP_OpsInterface = 
{
    .getIdx = HAL_CoreGetIndex,
    .bootApp = HAL_CoreBootApp,
    .enterApp = HAL_CoreEnterApp,
};
