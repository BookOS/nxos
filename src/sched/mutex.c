/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: mutex for thread
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-13     JasonHu           Init
 */

#include <sched/mutex.h>
#include <sched/sched.h>
#include <sched/thread.h>
#include <io/irq.h>

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

/**
 * forever: if true lock mutex forever, or not return NX_ETIMEOUT if lock falied
 */
NX_Error NX_MutexLock(NX_Mutex *mutex, NX_Bool forever)
{
    if (mutex == NX_NULL || mutex->magic != MUTEX_MAGIC)
    {
        return NX_EFAULT;
    }

    do
    {
        /* disable interrupt for locking per cpu */
        NX_UArch level = NX_IRQ_SaveLevel();

        /* spin lock for mutex */
        if (NX_SpinLock(&mutex->lock, NX_False) == NX_EOK)
        {
            NX_IRQ_RestoreLevel(level);
            break;
        }
        else
        {
            /* restore interrupt for unlocking per cpu */
            NX_IRQ_RestoreLevel(level);

            /* checkout timeout */
            if (forever == NX_False)
            {
                return NX_ETIMEOUT;
            }
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
