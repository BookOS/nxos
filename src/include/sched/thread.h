/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: NX_Thread for NXOS
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-7      JasonHu           Init
 */

#ifndef __SCHED_THREAD__
#define __SCHED_THREAD__

#include <utils/list.h>
#include <mods/time/timer.h>
#include <sched/spin.h>
#include <sched/process.h>

#ifdef CONFIG_NX_THREAD_NAME_LEN
#define NX_THREAD_NAME_LEN CONFIG_NX_THREAD_NAME_LEN
#else
#define NX_THREAD_NAME_LEN 32
#endif

#ifdef CONFIG_NX_THREAD_STACK_SIZE
#define NX_THREAD_STACK_SIZE_DEFAULT CONFIG_NX_THREAD_STACK_SIZE
#else
#define NX_THREAD_STACK_SIZE_DEFAULT 8192
#endif

typedef void (*NX_ThreadHandler)(void *arg);

enum NX_ThreadState
{
    NX_THREAD_INIT,
    NX_THREAD_READY,
    NX_THREAD_RUNNING,
    NX_THREAD_SLEEP,
    NX_THREAD_DEEPSLEEP,
    NX_THREAD_EXIT,
};
typedef enum NX_ThreadState NX_ThreadState;

struct NX_ThreadResource
{
    NX_Timer *sleepTimer;
    NX_Process *process;
};
typedef struct NX_ThreadResource NX_ThreadResource;

struct NX_Thread
{
    /* thread list */
    NX_List list;
    NX_List globalList;
    NX_List exitList;
    NX_List processList;    /* list for process */

    NX_Spin lock;  /* lock for thread */

    /* thread info */
    NX_ThreadState state;
    NX_I32 tid;     /* thread id, -1 means no alloc failed */
    NX_ThreadHandler handler;
    void *threadArg;
    char name[NX_THREAD_NAME_LEN];
    
    /* thread stack */
    NX_U8 *stackBase;  /* stack base */
    NX_Size stackSize; 
    NX_U8 *stack;      /* stack top */
    
    /* thread sched */
    NX_U32 timeslice;
    NX_U32 ticks;
    NX_U32 needSched;
    NX_U32 isTerminated;
    NX_UArch onCore;        /* thread on which core */
    NX_UArch coreAffinity;  /* thread would like to run on the core */

    /* thread resource */
    NX_ThreadResource resource;
};
typedef struct NX_Thread NX_Thread;

struct NX_ThreadManager
{
    NX_List globalList;    /* for global thread list */
    NX_List exitList;      /* for thread will exit soon */
    NX_List pendingList;   /* for thread ready to be pulled by the core */
    NX_Atomic averageThreadThreshold;    /* Average number of threads on core for load balance */
    NX_Atomic activeThreadCount;
    NX_Atomic pendingThreadCount;

    NX_Spin lock;    /* lock for thread manager */
    NX_Spin exitLock;    /* lock for thread exit */
};
typedef struct NX_ThreadManager NX_ThreadManager;

#define NX_CurrentThread NX_ThreadSelf()

NX_PUBLIC NX_Thread *NX_ThreadCreate(const char *name, NX_ThreadHandler handler, void *arg);
NX_PUBLIC NX_Error NX_ThreadDestroy(NX_Thread *thread);

NX_PUBLIC NX_Error NX_ThreadTerminate(NX_Thread *thread);
NX_PUBLIC void NX_ThreadExit(void);
NX_PUBLIC NX_Thread *NX_ThreadSelf(void);
NX_PUBLIC NX_Thread *NX_ThreadFindById(NX_U32 tid);

NX_PUBLIC NX_Error NX_ThreadRun(NX_Thread *thread);
NX_PUBLIC void NX_ThreadYield(void);
NX_PUBLIC NX_Error NX_ThreadSetAffinity(NX_Thread *thread, NX_UArch coreId);

NX_PUBLIC NX_Error NX_ThreadSleep(NX_UArch microseconds);
NX_PUBLIC NX_Error NX_ThreadWakeup(NX_Thread *thread);

NX_PUBLIC void NX_ThreadsInit(void);

/* thread sched */
NX_PUBLIC void NX_SchedToFirstThread(void);

NX_PUBLIC void NX_ThreadEnququeExitList(NX_Thread *thread);
NX_PUBLIC NX_Thread *NX_ThreadDeququeExitList(void);

NX_PUBLIC void NX_ThreadEnqueuePendingList(NX_Thread *thread);
NX_PUBLIC NX_Thread *NX_ThreadDequeuePendingList(void);

NX_PUBLIC void NX_ThreadReadyRunLocked(NX_Thread *thread, int flags);
NX_PUBLIC void NX_ThreadReadyRunUnlocked(NX_Thread *thread, int flags);

NX_PUBLIC void NX_ThreadExitProcess(NX_Thread *thread, NX_Process *process);

#endif /* __SCHED_THREAD__ */
