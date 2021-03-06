/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Log tools
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-12     JasonHu           Init
 */

#include <base/log.h>
#include <base/spin.h>

/* spin lock for log output */
NX_PRIVATE NX_SPIN_DEFINE_UNLOCKED(logOutputLock);

NX_Error LogLineLock(NX_UArch *level)
{
    return NX_SpinLockIRQ(&logOutputLock, level);
}

NX_Error LogLineUnlock(NX_UArch level)
{
    return NX_SpinUnlockIRQ(&logOutputLock, level);
}
