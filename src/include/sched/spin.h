/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: busy spin lock
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-13     JasonHu           Init
 */

#ifndef __SCHED_SPIN___
#define __SCHED_SPIN___

#include <xbook.h>
#include <xbook/atomic.h>

#define NX_SPIN_MAGIC 0x10000001
#define NX_SPIN_LOCK_VALUE 1

struct NX_Spin
{
    NX_Atomic value;
    NX_U32 magic;  /* magic for spin init */
};
typedef struct NX_Spin NX_Spin;

#define STATIC_SPIN_UNLOCKED(name) NX_Spin name = {NX_ATOMIC_INIT_VALUE(0), NX_SPIN_MAGIC}
#define STATIC_SPIN_LOCKED(name) NX_Spin name = {NX_ATOMIC_INIT_VALUE(1), NX_SPIN_MAGIC}

NX_Error NX_SpinInit(NX_Spin *lock);
NX_Error NX_SpinLock(NX_Spin *lock, NX_Bool forever);
NX_Error NX_SpinUnlock(NX_Spin *lock);
NX_Error NX_SpinLockIRQ(NX_Spin *lock, NX_UArch *level);
NX_Error NX_SpinUnlockIRQ(NX_Spin *lock, NX_UArch level);

#endif /* __SCHED_SPIN___ */
