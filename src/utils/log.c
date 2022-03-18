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

#include <utils/log.h>
#include <sched/spin.h>

/* spin lock for log output */
NX_PRIVATE NX_SPIN_DEFINE_UNLOCKED(LogOutputLock);

NX_Error LogLineLock(NX_UArch *level)
{
    return NX_SpinLockIRQ(&LogOutputLock, level);
}

NX_Error LogLineUnlock(NX_UArch level)
{
    return NX_SpinUnlockIRQ(&LogOutputLock, level);
}
