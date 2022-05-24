/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: mutex for thread
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-13     JasonHu           Init
 */

#include <base/mutex.h>
#include <base/sched.h>
#include <base/thread.h>
#include <base/irq.h>

#define MUTEX_MAGIC 0x10000002

NX_Error NX_MutexInit(NX_Mutex *mutex)
{
    if (mutex == NX_NULL)
    {
        return NX_EINVAL;
    }
    if (NX_SpinInit(&mutex->lock) != NX_EOK)
    {
        return NX_EPERM;
    }
    mutex->magic = MUTEX_MAGIC;
    return NX_EOK;
}

NX_Error NX_MutexInitLocked(NX_Mutex *mutex)
{
    if (mutex == NX_NULL)
    {
        return NX_EINVAL;
    }
    if (NX_SpinInit(&mutex->lock) != NX_EOK)
    {
        return NX_EPERM;
    }
    mutex->magic = MUTEX_MAGIC;
    NX_SpinLock(&mutex->lock);
    return NX_EOK;
}

NX_Error NX_MutexTryLock(NX_Mutex *mutex)
{
    if (mutex == NX_NULL || mutex->magic != MUTEX_MAGIC)
    {
        return NX_EFAULT;
    }

    /* disable interrupt for locking per cpu */
    NX_UArch level = NX_IRQ_SaveLevel();
    /* spin lock for mutex */
    NX_Error err = NX_SpinTryLock(&mutex->lock);
    NX_IRQ_RestoreLevel(level);
    return err;
}

NX_Error NX_MutexLock(NX_Mutex *mutex)
{
    if (mutex == NX_NULL || mutex->magic != MUTEX_MAGIC)
    {
        return NX_EFAULT;
    }

    do
    {
        /* spin lock for mutex */
        if (NX_MutexTryLock(mutex) == NX_EOK)
        {
            break;
        }
        else
        {
            /* yield to other thread */
            NX_ThreadYield();
        }
    } while (1);

    return NX_EOK;
}

NX_Error NX_MutexUnlock(NX_Mutex *mutex)
{
    if (mutex == NX_NULL || mutex->magic != MUTEX_MAGIC)
    {
        return NX_EFAULT;
    }
    NX_SpinUnlock(&mutex->lock);
    return NX_EOK;
}

NX_Error NX_MutexState(NX_Mutex *mutex)
{
    if (mutex == NX_NULL || mutex->magic != MUTEX_MAGIC)
    {
        return NX_EFAULT;
    }
    return NX_SpinState(&mutex->lock);
}
