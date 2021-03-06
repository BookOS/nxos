/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Thread for NXOS
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-7      JasonHu           Init
 */

#define NX_LOG_NAME "Thread"
#include <base/log.h>
#include <base/debug.h>
#include <base/irq.h>

#include <base/thread.h>
#include <base/thread_id.h>
#include <base/sched.h>
#include <base/mutex.h>
#include <base/smp.h>
#include <base/context.h>
#include <base/malloc.h>
#include <base/page.h>
#include <base/string.h>
#include <base/timer.h>

NX_ThreadManager gThreadManagerObject;

NX_PRIVATE NX_Error ThreadInit(NX_Thread *thread, 
    const char *name,
    NX_ThreadHandler handler, void *arg,
    NX_U8 *stack, NX_Size stackSize,
    NX_U32 priority)
{
    if (thread == NX_NULL || name == NX_NULL || handler == NX_NULL || stack == NX_NULL || !stackSize)
    {
        return NX_EINVAL;
    }

    NX_ListInit(&thread->list);
    NX_ListInit(&thread->globalList);
    NX_ListInit(&thread->exitList);
    NX_ListInit(&thread->processList);
    NX_ListInit(&thread->blockList);

    NX_StrCopy(thread->name, name);
    thread->tid = NX_ThreadIdAlloc();
    if (thread->tid < 0)
    {
        NX_LOG_E("No enough thread id.");
        return NX_ENORES;
    }
    thread->state = NX_THREAD_INIT;
    thread->handler = handler;
    thread->userHandler = NX_NULL;
    thread->threadArg = arg;
    thread->timeslice = 3;
    thread->elapsedTicks = 0;
    thread->ticks = thread->timeslice;
    thread->fixedPriority = priority;
    thread->priority = priority;
    thread->needSched = 0;
    thread->isTerminated = 0;
    thread->stackBase = stack;
    thread->stackSize = stackSize;
    thread->stack = thread->stackBase + stackSize - sizeof(NX_UArch);
    thread->stack = NX_ContextInit(handler, (void *)NX_ThreadExit, arg, thread->stack);
    thread->userStackBase = NX_NULL;
    thread->userStackSize = 0;

    thread->onCore = NX_MULTI_CORES_NR; /* not on any core */
    thread->coreAffinity = NX_MULTI_CORES_NR; /* no core affinity */

    thread->resource.sleepTimer = NX_NULL;
    thread->resource.process = NX_NULL;
    thread->resource.fileTable = NX_VfsGetDefaultFileTable();
    thread->resource.hub = NX_NULL;
    thread->resource.activeChannel = NX_NULL;
    thread->resource.exitCode = 0;
    thread->resource.waitExitCode = 0;
    NX_SemaphoreInit(&thread->resource.waiterSem, 0);
    thread->resource.tls = NX_NULL;

    NX_SpinInit(&thread->lock);
    return NX_EOK;
}

NX_PRIVATE NX_Error ThreadDeInit(NX_Thread *thread)
{
    if (thread == NX_NULL)
    {
        return NX_EINVAL;
    }
    if (thread->stackBase == NX_NULL)
    {
        return NX_EFAULT;
    }
    NX_ThreadIdFree(thread->tid);    
    return NX_EOK;
}

NX_Thread *NX_ThreadCreate(const char *name, NX_ThreadHandler handler, void *arg, NX_U32 priority)
{
    if (!name || !handler || priority >= NX_THREAD_MAX_PRIORITY_NR)
    {
        return NX_NULL;
    }

    NX_Thread *thread = (NX_Thread *)NX_MemAlloc(sizeof(NX_Thread));
    if (thread == NX_NULL)
    {
        return NX_NULL;
    }
    NX_U8 *stack = NX_MemAlloc(NX_THREAD_STACK_SIZE_DEFAULT);
    if (stack == NX_NULL)
    {
        NX_MemFree(thread);
        return NX_NULL;
    }
    if (ThreadInit(thread, name, handler, arg, stack, NX_THREAD_STACK_SIZE_DEFAULT, priority) != NX_EOK)
    {
        NX_MemFree(stack);
        NX_MemFree(thread);
        return NX_NULL;
    }
    return thread;
}

NX_Error NX_ThreadDestroy(NX_Thread *thread)
{
    if (thread == NX_NULL)
    {
        return NX_EINVAL;
    }
    NX_U8 *stackBase = thread->stackBase;
    if (stackBase == NX_NULL)
    {
        return NX_EFAULT;
    }

    NX_Error err = ThreadDeInit(thread);
    if (err != NX_EOK)
    {
        return err;
    }

    NX_MemFree(stackBase);
    NX_MemFree(thread);
    return NX_EOK;
}

NX_PRIVATE void NX_ThreadAddToGlobalPendingList(NX_Thread *thread, int flags)
{
    if (flags & NX_SCHED_HEAD)
    {
        NX_ListAdd(&thread->list, &gThreadManagerObject.pendingList);
    }
    else
    {
        NX_ListAddTail(&thread->list, &gThreadManagerObject.pendingList);
    }
    NX_AtomicInc(&gThreadManagerObject.pendingThreadCount);
}

NX_PRIVATE void NX_ThreadDelFromGlobalPendingList(NX_Thread *thread)
{
    NX_ListDel(&thread->list);
    NX_AtomicDec(&gThreadManagerObject.pendingThreadCount);
}

void NX_ThreadReadyRunLocked(NX_Thread *thread, int flags)
{
    thread->state = NX_THREAD_READY;

    if (thread->onCore < NX_MULTI_CORES_NR) /* add to core pending list */
    {
        NX_SMP_EnqueueThreadIrqDisabled(thread->onCore, thread, flags);
    }
    else /* add to global pending list */
    {
        NX_ThreadAddToGlobalPendingList(thread, flags);
    }
}

void NX_ThreadReadyRunUnlocked(NX_Thread *thread, int flags)
{
    NX_UArch level;
    NX_SpinLockIRQ(&gThreadManagerObject.lock, &level);

    NX_ThreadReadyRunLocked(thread, flags);
    
    NX_SpinUnlockIRQ(&gThreadManagerObject.lock, level);
}

void NX_ThreadUnreadyRunLocked(NX_Thread *thread)
{
    if (thread->onCore < NX_MULTI_CORES_NR) /* add to core pending list */
    {
        NX_SMP_DequeueThreadIrqDisabled(thread->onCore, thread);
    }
    else /* add to global pending list */
    {
        NX_ThreadDelFromGlobalPendingList(thread);
    }
}

void NX_ThreadUnreadyRun(NX_Thread *thread)
{
    NX_ASSERT(thread->state != NX_THREAD_READY);

    if (thread->onCore < NX_MULTI_CORES_NR) /* add to core pending list */
    {        
        NX_SMP_DequeueThread(thread->onCore, thread);
    }
    else /* add to global pending list */
    {
        NX_ThreadDequeuePendingList(thread);
    }
}

NX_INLINE void NX_ThreadEnququeGlobalListUnlocked(NX_Thread *thread)
{
    NX_ListAdd(&thread->globalList, &gThreadManagerObject.globalList);    
    NX_AtomicInc(&gThreadManagerObject.activeThreadCount);
}

NX_INLINE void NX_ThreadDeququeGlobalListUnlocked(NX_Thread *thread)
{
    NX_ListDel(&thread->globalList);
    NX_AtomicDec(&gThreadManagerObject.activeThreadCount);
}

NX_Error NX_ThreadStart(NX_Thread *thread)
{
    if (thread == NX_NULL)
    {
        return NX_EINVAL;
    }

    NX_UArch level;
    NX_SpinLockIRQ(&gThreadManagerObject.lock, &level);

    NX_ThreadEnququeGlobalListUnlocked(thread);

    /* add to ready list */
    NX_ThreadReadyRunLocked(thread, NX_SCHED_TAIL);
    
    NX_SpinUnlockIRQ(&gThreadManagerObject.lock, level);
    return NX_EOK;
}

NX_Error NX_ThreadStartNotReady(NX_Thread *thread)
{
    if (thread == NX_NULL)
    {
        return NX_EINVAL;
    }

    NX_UArch level;
    NX_SpinLockIRQ(&gThreadManagerObject.lock, &level);

    NX_ThreadEnququeGlobalListUnlocked(thread);

    NX_SpinUnlockIRQ(&gThreadManagerObject.lock, level);
    return NX_EOK;
}

void NX_ThreadYield(void)
{
    NX_SchedYield();
}

NX_Error NX_ThreadTerminate(NX_Thread *thread, NX_U32 exitCode)
{
    if (thread == NX_NULL)
    {
        return NX_EINVAL;
    }
    if (thread->state == NX_THREAD_INIT || thread->state == NX_THREAD_EXIT)
    {
        return NX_EPERM;
    }

    NX_UArch level = NX_IRQ_SaveLevel();
    thread->isTerminated = 1;
    thread->resource.exitCode = exitCode;
    NX_ThreadUnblock(thread);
    NX_IRQ_RestoreLevel(level);
    return NX_EOK;
}

NX_PRIVATE void ThreadExitNotify(NX_Thread * thread)
{
    NX_Thread * waiterThread;

    NX_ListForEachEntry(waiterThread, &thread->resource.waiterSem.semWaitList, blockList)
    {
        waiterThread->resource.waitExitCode = thread->resource.exitCode;
    }

    /* wakeup waiter */
    NX_SemaphoreSignalAll(&thread->resource.waiterSem);
}

/**
 * release resource the thread hold.
 */
NX_PRIVATE void ThreadReleaseResouce(NX_Thread *thread)
{
    /* NOTE: add other resource here. */
    if (thread->resource.sleepTimer != NX_NULL)
    {
        NX_TimerStop(thread->resource.sleepTimer);
        thread->resource.sleepTimer = NX_NULL;
    }

    /* thread exit notify */
    ThreadExitNotify(thread);

    /* thread had bind on process */
    if (thread->resource.process != NX_NULL)
    {
        NX_ThreadExitProcess(thread, thread->resource.process);
    }
}

void NX_ThreadExit(NX_U32 exitCode)
{
    /* free thread resource */
    NX_Thread *thread = NX_ThreadSelf();

    thread->resource.exitCode = exitCode;

    /* release the resource here that not the must for a thread! */
    ThreadReleaseResouce(thread);

    NX_UArch level;
    NX_SpinLockIRQ(&gThreadManagerObject.lock, &level);

    NX_ThreadDeququeGlobalListUnlocked(thread);

    NX_SpinUnlockIRQ(&gThreadManagerObject.lock, level);
    
    NX_SchedExit();
    NX_PANIC("Thread Exit should never arrive here!");
}

NX_Thread *NX_ThreadSelf(void)
{
    NX_Thread *cur = NX_SMP_GetRunning();
    NX_ASSERT(cur != NX_NULL);
    return cur;
}

/**
 * must called with interrupt disabled.
 */
NX_Error NX_ThreadBlockLockedIRQ(NX_Thread *thread, NX_Spin *lock, NX_UArch irqLevel)
{
    if (thread == NX_NULL)
    {
        return NX_EINVAL;
    }

    if (thread->state == NX_THREAD_READY)
    {   /* remove from ready list */
        thread->state = NX_THREAD_BLOCKED;
        NX_ThreadUnreadyRun(thread);

        if (lock)
        {
            NX_SpinUnlockIRQ(lock, irqLevel);
        }
        else
        {
            NX_IRQ_RestoreLevel(irqLevel);
        }
    }
    else if (thread->state == NX_THREAD_RUNNING)
    {   /* set as block, then sched */
        thread->state = NX_THREAD_BLOCKED;
        NX_SchedLockedIRQ(irqLevel, lock);
        
        if (thread->isTerminated != 0) /* check exit */
        {
            NX_ThreadExit(1);
        }
    }
    
    return NX_EOK;
}

/**
 * must called when interrupt disabled
 */
NX_Error NX_ThreadBlockInterruptDisabled(NX_Thread *thread, NX_UArch irqLevel)
{
    return NX_ThreadBlockLockedIRQ(thread, NX_NULL, irqLevel);
}

NX_Error NX_ThreadBlock(NX_Thread *thread)
{
    NX_UArch level = NX_IRQ_SaveLevel();
    return NX_ThreadBlockLockedIRQ(thread, NX_NULL, level);
}

/**
 * wakeup a thread, must called interrupt disabled
 */
NX_Error NX_ThreadUnblock(NX_Thread *thread)
{
    if (thread == NX_NULL)
    {
        return NX_EINVAL;
    }

    /* if thread in sleep, then wakeup it */
    if (thread->state == NX_THREAD_BLOCKED)
    {
        NX_ThreadReadyRunUnlocked(thread, NX_SCHED_HEAD);
        return NX_EOK;
    }

    return NX_EBUSY;
}

NX_PRIVATE NX_Bool TimerThreadSleepTimeout(NX_Timer *timer, void *arg)
{
    NX_Thread *thread = (NX_Thread *)arg; /* the thread wait for timeout  */

    thread->resource.sleepTimer = NX_NULL; /* cleanup sleep timer */

    if (NX_ThreadUnblock(thread) != NX_EOK)
    {
        NX_LOG_E("Wakeup thread:%s/%d failed!", thread->name, thread->tid);
    }
    
    return NX_True;
}

/* if thread sleep less equal than 2ms, use delay instead */
#define THREAD_SLEEP_TIMEOUT_THRESHOLD 2

NX_Error NX_ThreadSleep(NX_UArch microseconds)
{
    if (microseconds == 0)
    {
        return NX_EINVAL;
    }
    if (microseconds <= THREAD_SLEEP_TIMEOUT_THRESHOLD)
    {
        return NX_ClockTickDelayMillisecond(microseconds);
    }

    NX_Timer sleepTimer;
    NX_Error err;

    NX_UArch irqLevel = NX_IRQ_SaveLevel();
    NX_Thread *self = NX_ThreadSelf();

    /* must exit if terminated */
    if (self->isTerminated != 0)
    {
        NX_ThreadExit(1);
        NX_PANIC("thread sleep was terminate but exit failed");
    }

    err = NX_TimerInit(&sleepTimer, microseconds, TimerThreadSleepTimeout, (void *)self, NX_TIMER_ONESHOT);
    if (err != NX_EOK)
    {
        NX_IRQ_RestoreLevel(irqLevel);
        return err;
    }

    self->resource.sleepTimer = &sleepTimer;

    NX_TimerStart(self->resource.sleepTimer);

    /* set thread as sleep state */
    NX_ThreadBlockInterruptDisabled(self, irqLevel);

    /* if sleep timer always here, it means that the thread was interrupted! */
    if (self->resource.sleepTimer != NX_NULL)
    {
        /* timer not stop now */
        NX_TimerStop(self->resource.sleepTimer);
        self->resource.sleepTimer = NX_NULL;

        /* must exit if terminated */
        if (self->isTerminated != 0)
        {
            NX_ThreadExit(1);
            NX_PANIC("thread sleep was terminate but exit failed");
        }
        return NX_EINTR;
    }
    return NX_EOK;
}

NX_Error NX_ThreadSetAffinity(NX_Thread *thread, NX_UArch coreId)
{
    if (thread == NX_NULL || coreId >= NX_MULTI_CORES_NR)
    {
        return NX_EINVAL;
    }
    NX_UArch level;
    NX_SpinLockIRQ(&thread->lock, &level);
    thread->coreAffinity = coreId;
    thread->onCore = coreId;
    NX_SpinUnlockIRQ(&thread->lock, level);
    return NX_EOK;
}

void NX_ThreadEnqueuePendingList(NX_Thread *thread)
{
    NX_UArch level;
    NX_SpinLockIRQ(&gThreadManagerObject.lock, &level);
    NX_ThreadAddToGlobalPendingList(thread, NX_SCHED_HEAD);
    NX_SpinUnlockIRQ(&gThreadManagerObject.lock, level);
}

void NX_ThreadDequeuePendingList(NX_Thread *thread)
{
    NX_UArch level;
    NX_SpinLockIRQ(&gThreadManagerObject.lock, &level);
    NX_ThreadDelFromGlobalPendingList(thread);
    NX_SpinUnlockIRQ(&gThreadManagerObject.lock, level);
}

NX_Thread *NX_ThreadPickPendingList(void)
{
    NX_Thread *thread;
    NX_SpinLock(&gThreadManagerObject.lock);
    thread = NX_ListFirstEntryOrNULL(&gThreadManagerObject.pendingList, NX_Thread, list);
    if (thread != NX_NULL)
    {
        NX_ListDel(&thread->list);
        NX_AtomicDec(&gThreadManagerObject.pendingThreadCount);
    }
    NX_SpinUnlock(&gThreadManagerObject.lock);
    return thread;
}

void NX_ThreadEnququeExitList(NX_Thread *thread)
{
    NX_UArch level;
    NX_SpinLockIRQ(&gThreadManagerObject.exitLock, &level);
    NX_ASSERT(!NX_ListFind(&thread->exitList, &gThreadManagerObject.exitList));
    NX_ListAdd(&thread->exitList, &gThreadManagerObject.exitList);
    NX_SpinUnlockIRQ(&gThreadManagerObject.exitLock, level);
}

NX_Thread *NX_ThreadDeququeExitList(void)
{
    NX_Thread *thread;
    NX_UArch level;
    NX_SpinLockIRQ(&gThreadManagerObject.exitLock, &level);
    thread = NX_ListFirstEntryOrNULL(&gThreadManagerObject.exitList, NX_Thread, exitList);
    if (thread != NX_NULL)
    {
        NX_ListDelInit(&thread->exitList);
        NX_ASSERT(!NX_ListFind(&thread->exitList, &gThreadManagerObject.exitList));
    }
    NX_SpinUnlockIRQ(&gThreadManagerObject.exitLock, level);
    return thread;
}

NX_Thread *NX_ThreadFindById(NX_U32 tid)
{
    NX_Thread *thread = NX_NULL, *find = NX_NULL;
    NX_UArch level;

    NX_SpinLockIRQ(&gThreadManagerObject.lock, &level);

    NX_ListForEachEntry (thread, &gThreadManagerObject.globalList, globalList)
    {
        if (thread->tid == tid)
        {
            find = thread;
            break;
        }
    }

    NX_SpinUnlockIRQ(&gThreadManagerObject.lock, level);
    return find;
}

NX_Error NX_ThreadWalk(NX_ThreadWalkHandler handler, void * arg)
{
    NX_Thread *thread;
    NX_UArch level;
    NX_Error err = NX_EOK;

    if (!handler)
    {
        return NX_EINVAL;
    }

    NX_SpinLockIRQ(&gThreadManagerObject.lock, &level);

    NX_ListForEachEntry (thread, &gThreadManagerObject.globalList, globalList)
    {
        if ((err = handler(thread, arg)) != NX_EOK)
        {
            break;
        }
    }

    NX_SpinUnlockIRQ(&gThreadManagerObject.lock, level);

    return err;
}

NX_Error NX_ThreadWait(NX_Thread * thread, NX_U32 *exitCode)
{
    NX_Thread * self;
    
    if (thread == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    self = NX_ThreadSelf();

    if (thread == self)
    {
        NX_LOG_E("thread %s/%d wait self!", thread->tid, thread->name);
        return NX_EINVAL;
    }

    /* wait on waiters sem */
    NX_SemaphoreWait(&thread->resource.waiterSem);
    if (exitCode)
    {
        *exitCode = self->resource.waitExitCode;
    }

    return NX_EOK;
}

void NX_ThreadManagerInit(void)
{
    NX_AtomicSet(&gThreadManagerObject.averageThreadThreshold, 0);
    NX_AtomicSet(&gThreadManagerObject.activeThreadCount, 0);
    NX_AtomicSet(&gThreadManagerObject.pendingThreadCount, 0);
    NX_ListInit(&gThreadManagerObject.exitList);
    NX_ListInit(&gThreadManagerObject.globalList);
    NX_ListInit(&gThreadManagerObject.pendingList);
    
    NX_SpinInit(&gThreadManagerObject.lock);
    NX_SpinInit(&gThreadManagerObject.exitLock);
}

NX_IMPORT void NX_ThreadInitIdle(void);
NX_IMPORT void NX_ThreadInitDeamon(void);

void NX_ThreadsInit(void)
{
    NX_ThreadsInitID();
    NX_ThreadManagerInit();
    NX_ThreadInitIdle();
    NX_ThreadInitDeamon();
}
