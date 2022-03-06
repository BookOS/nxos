/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: NX_Thread test
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-13     JasonHu           Init
 */

#define NX_LOG_NAME "Thread"
#include <utils/log.h>

#include <xbook/debug.h>
#include <sched/thread.h>
#include <mods/test/integration.h>

#ifdef CONFIG_NX_TEST_INTEGRATION_THREAD

NX_PRIVATE NX_VOLATILE int threadTick = 0;

NX_PRIVATE void TestThread1(void *arg)
{
    NX_LOG_I("Hello, test thread 1: %p", arg);
    // NX_Thread *self = NX_ThreadSelf();
    int i = 0;
    while (1)
    {
        i++;
        if (i > 100)
        {
            threadTick++;
            NX_ThreadExit();
        }
    }
}

NX_PRIVATE void TestThread2(void *arg)
{
    NX_LOG_I("Hello, test thread 2: %p", arg);
    
    NX_Thread *self = NX_ThreadSelf();
    int i = 0;
    while (1)
    {
        i++;
        if (i > 100)
        {
            threadTick++;
            break;
        }
    }
    NX_LOG_I("thread exit: %s", self->name);
}

NX_PRIVATE void TestThread3(void *arg)
{
    NX_LOG_I("Hello, test thread 3: %p", arg);
    
    /* wait terminate */
    while (1)
    {
        
    }
}

NX_PRIVATE NX_U32 thread3ID;

NX_PRIVATE void TestThread4(void *arg)
{
    NX_LOG_I("Hello, test thread 4: %p", arg);
    NX_Thread *target = NX_ThreadFindById(thread3ID);
    NX_ASSERT(target != NX_NULL);
    int i = 0;
    while (1)
    {
        i++;
        if (i == 100)
        {
            NX_LOG_D("terminate thread: %d", target->tid);
            NX_ThreadTerminate(target);
        }
        if (i == 1000)
        {
            threadTick++;
            return;
        }
    }
}

#include <sched/mutex.h>

NX_PRIVATE NX_Mutex mutexLock;

NX_PRIVATE void TestMutex1(void *arg)
{
    int i = 0;
    while (1)
    {
        i++;
        if (i > 100)
        {
            threadTick++;
            return;
        }
        NX_MutexLock(&mutexLock, NX_True);
        NX_LOG_I(NX_ThreadSelf()->name, " get lock");
        NX_MutexUnlock(&mutexLock);
        NX_ClockTickDelayMillisecond(10);
    }
}

NX_INTEGRATION_TEST(TestThread)
{
    /* thread */
    threadTick = 0;
    NX_Thread *thread = NX_ThreadCreate("test thread 1", TestThread1, (void *) 0x1234abcd);
    /* make sure has enough memory */
    if (thread == NX_NULL)
    {
        return NX_ENOMEM;
    }
    NX_ASSERT(thread != NX_NULL);
    NX_ASSERT(NX_ThreadRun(thread) == NX_EOK);

    thread = NX_ThreadCreate("test thread 2", TestThread2, (void *) 0x1234abcd);
    /* make sure has enough memory */
    if (thread == NX_NULL)
    {
        return NX_ENOMEM;
    }
    NX_ASSERT(thread != NX_NULL);
    NX_ASSERT(NX_ThreadRun(thread) == NX_EOK);

    thread = NX_ThreadCreate("test thread 3", TestThread3, (void *) 0x1234abcd);
    /* make sure has enough memory */
    if (thread == NX_NULL)
    {
        return NX_ENOMEM;
    }
    NX_ASSERT(thread != NX_NULL);
    NX_ASSERT(NX_ThreadRun(thread) == NX_EOK);

    thread3ID = thread->tid;
    
    thread = NX_ThreadCreate("test thread 4", TestThread4, (void *) 0x1234abcd);
    NX_ASSERT(thread != NX_NULL);
    NX_ASSERT(NX_ThreadRun(thread) == NX_EOK);

    while (threadTick != 3);

    /* mutex */
    threadTick = 0;

    NX_MutexInit(&mutexLock);
    NX_Thread *mutexThread = NX_ThreadCreate("mutex thread 1", TestMutex1, NX_NULL);
    NX_ASSERT(mutexThread != NX_NULL);
    NX_ASSERT(NX_ThreadRun(mutexThread) == NX_EOK);

    mutexThread = NX_ThreadCreate("mutex thread 2", TestMutex1, NX_NULL);
    NX_ASSERT(mutexThread != NX_NULL);
    NX_ASSERT(NX_ThreadRun(mutexThread) == NX_EOK);
    
    mutexThread = NX_ThreadCreate("mutex thread 3", TestMutex1, NX_NULL);
    NX_ASSERT(mutexThread != NX_NULL);
    NX_ASSERT(NX_ThreadRun(mutexThread) == NX_EOK);

    while (threadTick != 3);

    NX_LOG_D("thread test done.");    
    return NX_EOK;
}

#endif
