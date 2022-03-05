/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: spin lock
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-21     JasonHu           Init
 */

#include <sched/spin.h>
#include <io/irq.h>

NX_Error NX_SpinInit(NX_Spin *lock)
{
    if (lock == NX_NULL)
    {
        return NX_EINVAL;
    }
    if (lock->magic == NX_SPIN_MAGIC)
    {
        return NX_EFAULT;
    }

    NX_AtomicSet(&lock->value, 0);
    lock->magic = NX_SPIN_MAGIC;
    return NX_EOK;
}

NX_Error NX_SpinLock(NX_Spin *lock, NX_Bool forever)
{
    if (lock == NX_NULL || lock->magic != NX_SPIN_MAGIC)
    {
        return NX_EFAULT;
    }

    do
    {
        if (NX_AtomicCAS(&lock->value, 0, NX_SPIN_LOCK_VALUE) == 0)
        {
            break;
        }
        if (forever == NX_False)
        {
            return NX_ETIMEOUT;
        }
    } while (1);

    return NX_EOK;
}

NX_Error NX_SpinUnlock(NX_Spin *lock)
{
    if (lock == NX_NULL || lock->magic != NX_SPIN_MAGIC)
    {
        return NX_EFAULT;
    }
    NX_AtomicSet(&lock->value, 0);
    return NX_EOK;
}

NX_Error NX_SpinLockIRQ(NX_Spin *lock, NX_UArch *level)
{
    if (lock == NX_NULL || level == NX_NULL)
    {
        return NX_EINVAL;
    }
    *level = NX_IRQ_SaveLevel();
    return NX_SpinLock(lock, NX_True);
}

NX_Error NX_SpinUnlockIRQ(NX_Spin *lock, NX_UArch level)
{
    if (lock == NX_NULL)
    {
        return NX_EINVAL;
    }
    if (NX_SpinUnlock(lock) != NX_EOK)
    {
        return NX_EFAULT;
    }
    NX_IRQ_RestoreLevel(level);
    return NX_EOK;
}
