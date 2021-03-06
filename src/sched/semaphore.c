/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: semaphore
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-9       JasonHu           Init
 */

#include <base/semaphore.h>
#include <base/thread.h>
#include <base/debug.h>
#include <base/log.h>

#define SEMPAHORE_MAGIC 0x10000003

NX_Error NX_SemaphoreInit(NX_Semaphore *sem, NX_IArch value)
{
    if (!sem)
    {
        return NX_EINVAL;
    }
    NX_AtomicSet(&sem->value, value);
    NX_SpinInit(&sem->lock);
    NX_ListInit(&sem->semWaitList);
    sem->magic = SEMPAHORE_MAGIC;
    return NX_EOK;
}

NX_IArch NX_SemaphoreGetValue(NX_Semaphore *sem)
{
    if (!sem)
    {
        return 0;
    }
    return sem->value.value;
}

NX_Error NX_SemaphoreWait(NX_Semaphore *sem)
{
    NX_Thread *cur;
    NX_UArch level;

    if (!sem)
    {
        return NX_EINVAL;
    }

    if (sem->magic != SEMPAHORE_MAGIC)
    {
        return NX_EFAULT;
    }

    while (1)
    {
        NX_SpinLockIRQ(&sem->lock, &level);
        
        if (NX_AtomicGet(&sem->value) > 0) /* wait success */
        {
            NX_AtomicDec(&sem->value);
            NX_SpinUnlockIRQ(&sem->lock, level);
            break;
        }

        cur = NX_ThreadSelf();
        if (NX_ListEmptyCareful(&cur->blockList))
        {
            NX_ListAddTail(&cur->blockList, &sem->semWaitList); /* add to list tail */
        }

        NX_ASSERT(NX_ThreadBlockLockedIRQ(cur, &sem->lock, level) == NX_EOK); /* block self */
    }
    
    return NX_EOK;
}

NX_Error NX_SemaphoreTryWait(NX_Semaphore *sem)
{
    NX_Error err;
    NX_UArch level;

    if (!sem)
    {
        return NX_EINVAL;
    }

    if (sem->magic != SEMPAHORE_MAGIC)
    {
        return NX_EFAULT;
    }

    err = NX_EAGAIN;
    NX_SpinLockIRQ(&sem->lock, &level);
    
    if (NX_AtomicGet(&sem->value) > 0) /* wait success */
    {
        NX_AtomicDec(&sem->value);
        err = NX_EOK;
    }
    NX_SpinUnlockIRQ(&sem->lock, level);

    return err;
}

NX_Error NX_SemaphoreSignal(NX_Semaphore *sem)
{
    NX_Thread *thread, *next;
    NX_UArch level;

    if (!sem)
    {
        return NX_EINVAL;
    }

    if (sem->magic != SEMPAHORE_MAGIC)
    {
        return NX_EFAULT;
    }

    NX_SpinLockIRQ(&sem->lock, &level);

    NX_AtomicInc(&sem->value);

    if (NX_AtomicGet(&sem->value) > 0)
    {
        if (!NX_ListEmpty(&sem->semWaitList))
        {
            /* wakeup first thread */
            NX_ListForEachEntrySafe(thread, next, &sem->semWaitList, blockList)
            {
                NX_ListDelInit(&thread->blockList);

                NX_ThreadUnblock(thread);
                break;
            }
        }
    }

    NX_SpinUnlockIRQ(&sem->lock, level);
    return NX_EOK;
}

NX_Error NX_SemaphoreSignalAll(NX_Semaphore *sem)
{
    NX_Thread *thread, *next;
    NX_UArch level;

    if (!sem)
    {
        return NX_EINVAL;
    }

    if (sem->magic != SEMPAHORE_MAGIC)
    {
        return NX_EFAULT;
    }

    NX_SpinLockIRQ(&sem->lock, &level);

    NX_AtomicInc(&sem->value);

    if (NX_AtomicGet(&sem->value) > 0)
    {
        if (!NX_ListEmpty(&sem->semWaitList))
        {
            /* wakeup all thread */
            NX_ListForEachEntrySafe(thread, next, &sem->semWaitList, blockList)
            {
                NX_ListDelInit(&thread->blockList);

                NX_ThreadUnblock(thread);
            }
        }
    }

    NX_SpinUnlockIRQ(&sem->lock, level);
    return NX_EOK;
}

NX_Error NX_SemaphoreState(NX_Semaphore *sem)
{
    if (!sem)
    {
        return NX_EINVAL;
    }

    if (sem->magic != SEMPAHORE_MAGIC)
    {
        return NX_EFAULT;
    }

    if (NX_AtomicGet(&sem->value) == 0) /* has waiters */
    {
        return NX_EBUSY;
    }
    return NX_EOK;
}
