/**
 * Copyright (c) 2018-2022, BookOS Development Team
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
NX_PRIVATE STATIC_SPIN_UNLOCKED(LogOutputLock);

NX_Error LogLineLock(NX_UArch *level)
{
    return NX_SpinLockIRQ(&LogOutputLock, level);
}

NX_Error LogLineUnlock(NX_UArch level)
{
    return NX_SpinUnlockIRQ(&LogOutputLock, level);
}
