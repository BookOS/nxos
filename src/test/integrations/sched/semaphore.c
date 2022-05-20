/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: integrations for semaphore test 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-12      JasonHu           Init
 */

#include <sched/semaphore.h>
#include <sched/thread.h>
#define NX_LOG_NAME "semaphore"
#define NX_LOG_LEVEL NX_LOG_DBG
#include <utils/log.h>
#include <test/integration.h>

#ifdef CONFIG_NX_TEST_INTEGRATION_SEMAPHORE

/* Producer And Consumer test */
#define PC_TEST_TIME 10 /* 10 s */

#define MAX_FULL_SEM_VAL 10

/* config */
#define USE_SEM_MUTEX 1

#if USE_SEM_MUTEX == 1
NX_PRIVATE NX_Semaphore PC_Mutex;
#else
#include <sched/mutex.h>
NX_PRIVATE NX_Mutex PC_Mutex;
#endif

NX_PRIVATE NX_Semaphore PC_Full;
NX_PRIVATE NX_Semaphore PC_Empty;

NX_PRIVATE int goods = 0;

NX_PRIVATE void ProducerThread(void *arg)
{
    int temp = 0;
    while (1)
    {
        /* make goods */
        temp++;

        NX_LOG_D("make goods: %d", temp);

        NX_SemaphoreWait(&PC_Empty);

#if USE_SEM_MUTEX == 1
        NX_SemaphoreWait(&PC_Mutex);
#else
        NX_MutexLock(&PC_Mutex);
#endif

        /* put goods */
        goods = temp;

#if USE_SEM_MUTEX == 1
        NX_SemaphoreSignal(&PC_Mutex);
#else
        NX_MutexUnlock(&PC_Mutex);
#endif

        NX_SemaphoreSignal(&PC_Full); /* add a goods */
    }
}

NX_PRIVATE void ConsumerThread(void *arg)
{
    int temp = 0;

    while (1)
    {
        NX_SemaphoreWait(&PC_Full);

#if USE_SEM_MUTEX == 1
        NX_SemaphoreWait(&PC_Mutex);
#else
        NX_MutexLock(&PC_Mutex);
#endif

        /* get goods */
        temp = goods;

#if USE_SEM_MUTEX == 1
        NX_SemaphoreSignal(&PC_Mutex);
#else
        NX_MutexUnlock(&PC_Mutex);
#endif

        NX_SemaphoreSignal(&PC_Empty); /* del a goods */
        
        /* use goods */
        NX_LOG_D("use goods: %d", temp);

    }
}

NX_PRIVATE void ProducerAndConsumer(void)
{
    int sleepCount = 0;
    NX_Error err;
    
    NX_Atomic PC_State;

#if USE_SEM_MUTEX == 1
    NX_SemaphoreInit(&PC_Mutex, 1);
#else
    NX_MutexInit(&PC_Mutex);
#endif    

    NX_SemaphoreInit(&PC_Empty, MAX_FULL_SEM_VAL);
    NX_SemaphoreInit(&PC_Full, 0);
    
    NX_LOG_I("mutex sem:%p", &PC_Mutex);
    NX_LOG_I("empty sem:%p", &PC_Empty);
    NX_LOG_I("full sem:%p", &PC_Full);

    NX_AtomicSet(&PC_State, 0);

    NX_Thread *producer = NX_ThreadCreate("producer", ProducerThread, NX_NULL, NX_THREAD_PRIORITY_NORMAL);
    if (!producer)
    {
        NX_LOG_E("create producer error!");
        return;
    }
    NX_Thread *consumer = NX_ThreadCreate("consumer", ConsumerThread, NX_NULL, NX_THREAD_PRIORITY_NORMAL);
    if (!consumer)
    {
        NX_LOG_E("create consumer error!");
        return;
    }

    NX_ThreadStart(producer);
    NX_ThreadStart(consumer);
    
    while (NX_AtomicGet(&PC_State) != 1)
    {
        NX_LOG_I("sleep 1s");
        NX_ThreadSleep(1000); /* sleep 1s */
        sleepCount++;
        if (sleepCount == PC_TEST_TIME)
        {
            err = NX_ThreadTerminate(producer, 0);
            NX_LOG_I("terminate thread %s state: %d, err state: %s", producer->name, producer->state, NX_ErrorToString(err));
            err = NX_ThreadTerminate(consumer, 0);
            NX_LOG_I("terminate thread %s state: %d, err state: %s", consumer->name, consumer->state, NX_ErrorToString(err));
        }
        if (sleepCount == PC_TEST_TIME + 1)
        {
            NX_AtomicInc(&PC_State);
        }
    }
    NX_LOG_I("test ProducerAndConsumer ok!");
}

NX_INTEGRATION_TEST(NX_Semaphore)
{
    ProducerAndConsumer();
    return NX_EOK;
}

#endif
