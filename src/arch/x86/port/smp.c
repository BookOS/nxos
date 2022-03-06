/**
 * Copyright (c) 2018-2022, NXOS Development Team
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

NX_UArch NX_HalCoreGetIndex(void)
{
    return 0;
}

NX_Error NX_HalCoreBootApp(NX_UArch bootCoreId)
{
    NX_LOG_I("boot core is:%d", bootCoreId);
    return NX_ENORES;
}

NX_Error NX_HalCoreEnterApp(NX_UArch appCoreId)
{
    return NX_ENORES;
}

NX_INTERFACE struct NX_SMP_Ops NX_SMP_OpsInterface = 
{
    .getIdx = NX_HalCoreGetIndex,
    .bootApp = NX_HalCoreBootApp,
    .enterApp = NX_HalCoreEnterApp,
};
