/**
 * Copyright (c) 2018-2021, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Thread for NXOS
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-7      JasonHu           Init
 */

#ifndef __SCHED_THREAD__
#define __SCHED_THREAD__

#include <Utils/List.h>
#include <Mods/Time/Timer.h>

#ifdef CONFIG_THREAD_NAME_LEN
#define THREAD_NAME_LEN CONFIG_THREAD_NAME_LEN
#else
#define THREAD_NAME_LEN 32
#endif

#ifdef CONFIG_THREAD_STACK_SIZE
#define THREAD_STACK_SIZE_DEFAULT CONFIG_THREAD_STACK_SIZE
#else
#define THREAD_STACK_SIZE_DEFAULT 4096
#endif

typedef void (*ThreadHandler)(void *arg);

enum ThreadState
{
    THREAD_INIT,
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_SLEEP,
    THREAD_DEEPSLEEP,
    THREAD_EXIT,
};
typedef enum ThreadState ThreadState;

struct ThreadResource
{
    Timer *sleepTimer;
};
typedef struct ThreadResource ThreadResource;

struct Thread
{
    /* thread list */
    List list;
    List globalList;

    /* thread info */
    ThreadState state;
    Uint32 tid;     /* thread id */
    ThreadHandler handler;
    void *threadArg;
    char name[THREAD_NAME_LEN];
    
    /* thread stack */
    U8 *stackBase;  /* stack base */
    Size stackSize; 
    U8 *stack;      /* stack top */
    
    /* thread sched */
    U32 timeslice;
    U32 ticks;
    U32 needSched;
    U32 isTerminated;

    /* thread resource */
    ThreadResource resource;
};
typedef struct Thread Thread;

PUBLIC Thread *ThreadCreate(const char *name, ThreadHandler handler, void *arg);
PUBLIC OS_Error ThreadDestroy(Thread *thread);

PUBLIC OS_Error ThreadTerminate(Thread *thread);
PUBLIC void ThreadExit(void);
PUBLIC Thread *ThreadSelf(void);
PUBLIC Thread *ThreadFindById(U32 tid);

PUBLIC OS_Error ThreadRun(Thread *thread);
PUBLIC void ThreadYield(void);

PUBLIC OS_Error ThreadSleep(Uint microseconds);
PUBLIC OS_Error ThreadWakeup(Thread *thread);

PUBLIC void ThreadsInit(void);
PUBLIC void TestThread(void);

PUBLIC void SchedToFirstThread(void);

#endif /* __SCHED_THREAD__ */
