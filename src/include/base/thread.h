/**
 * Copyright (c) 2018-2022, NXOS Development Team
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

#include <base/list.h>
#include <base/timer.h>
#include <base/spin.h>
#include <base/semaphore.h>
#include <base/process.h>
#include <base/vfs.h>
#include <base/signal.h>

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

/* time-sharing priority */
#define NX_THREAD_PRIORITY_IDLE     0   /* idle thread priority */
#define NX_THREAD_PRIORITY_LOW      1   /* low level priority */
#define NX_THREAD_PRIORITY_NORMAL   3   /* normal level priority */
#define NX_THREAD_PRIORITY_HIGH     6   /* high level priority */

/* max thread priority */
#ifdef CONFIG_NX_THREAD_MAX_PRIORITY_NR
#define NX_THREAD_MAX_PRIORITY_NR CONFIG_NX_THREAD_MAX_PRIORITY_NR
#else
#define NX_THREAD_MAX_PRIORITY_NR 16
#endif

#define NX_THREAD_PRIORITY_RT_MIN (NX_THREAD_PRIORITY_HIGH + 1)     /* real time min priority */
#define NX_THREAD_PRIORITY_RT_MAX (NX_THREAD_MAX_PRIORITY_NR - 1)   /* real time max priority */

#define NX_THREAD_CREATE_SUSPEND 0x01 /* thread create with suspend flag */

typedef void (*NX_ThreadHandler)(void *arg);

enum NX_ThreadState
{
    NX_THREAD_INIT = 0,
    NX_THREAD_READY,
    NX_THREAD_RUNNING,
    NX_THREAD_BLOCKED,
    NX_THREAD_EXIT,
};
typedef enum NX_ThreadState NX_ThreadState;

struct NX_Hub;
struct NX_HubChannel;
struct NX_ThreadResource
{
    NX_Timer *sleepTimer;
    NX_Process *process;
    NX_VfsFileTable *fileTable;
    struct NX_Hub *hub; /* hub for each thread */
    struct NX_HubChannel *activeChannel; /* channel for this thread */
    
    NX_U32 exitCode;       /* exit code for thread */
    NX_U32 waitExitCode;   /* exit code for this thread wait another thread */
    NX_Semaphore waiterSem; /* The semaphore of the thread waiting for this thread to exit */
    void * tls; /* thread local storage */
    NX_SignalTable signals;
};
typedef struct NX_ThreadResource NX_ThreadResource;

struct NX_Thread
{
    /* thread list */
    NX_List list;   /* ready list */
    NX_List globalList;
    NX_List exitList;
    NX_List processList;    /* list for process */
    NX_List blockList;    /* list for block on somewhere */

    NX_Spin lock;  /* lock for thread */

    /* thread info */
    NX_ThreadState state;
    NX_I32 tid;     /* thread id, -1 means no alloc failed */
    NX_ThreadHandler handler;
    NX_ThreadHandler userHandler; /* user mode handler */
    void *threadArg;
    char name[NX_THREAD_NAME_LEN];
    
    /* thread stack */
    NX_U8 *stackBase;  /* stack base */
    NX_Size stackSize; 
    NX_U8 *stack;      /* stack top */
    NX_U8 *userStackBase;  /* user thread stack base */
    NX_Size userStackSize; /* user thread stack size */

    /* thread sched */
    NX_U32 timeslice;
    NX_U32 ticks;
    NX_ClockTick elapsedTicks; /* thread runs ticks */
    NX_U32 fixedPriority;  /* fixed priority, does not change dynamically  */
    NX_U32 priority;    /* dynamic priority, or in the case of time-sharing scheduling priority will change dynamically */
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

typedef NX_Error (* NX_ThreadWalkHandler)(NX_Thread * thread, void * arg);

typedef struct
{
    NX_Size stackSize;
    NX_U32 schedPriority;
} NX_ThreadAttr;

#define NX_CurrentThread NX_ThreadSelf()

#define NX_ThreadSetFileTable(thread, fileTable) ((thread)->resource.fileTable = fileTable)
#define NX_ThreadGetFileTable(thread) ((thread)->resource.fileTable)

#define NX_ThreadSetProcess(thread, process) ((thread)->resource.process = process)
#define NX_ThreadGetProcess(thread) ((thread)->resource.process)

NX_Thread *NX_ThreadCreate(const char *name, NX_ThreadHandler handler, void *arg, NX_U32 priority);
NX_Error NX_ThreadDestroy(NX_Thread *thread);

NX_Error NX_ThreadTerminate(NX_Thread *thread, NX_U32 exitCode);
void NX_ThreadExit(NX_U32 exitCode);
NX_Thread *NX_ThreadSelf(void);
NX_Thread *NX_ThreadFindById(NX_U32 tid);

NX_Error NX_ThreadStart(NX_Thread *thread);
NX_Error NX_ThreadStartNotReady(NX_Thread *thread);

void NX_ThreadYield(void);
NX_Error NX_ThreadSetAffinity(NX_Thread *thread, NX_UArch coreId);

NX_Error NX_ThreadBlock(NX_Thread *thread);
NX_Error NX_ThreadBlockInterruptDisabled(NX_Thread *thread, NX_UArch irqLevel);
NX_Error NX_ThreadBlockLockedIRQ(NX_Thread *thread, NX_Spin *lock, NX_UArch irqLevel);

NX_Error NX_ThreadUnblock(NX_Thread *thread);

NX_Error NX_ThreadSleep(NX_UArch microseconds);
NX_Error NX_ThreadWait(NX_Thread * thread, NX_U32 *exitCode);

NX_Error NX_ThreadWalk(NX_ThreadWalkHandler handler, void * arg);

void NX_ThreadsInit(void);

/* thread sched */
void NX_SchedToFirstThread(void);

void NX_ThreadEnququeExitList(NX_Thread *thread);
NX_Thread *NX_ThreadDeququeExitList(void);

void NX_ThreadEnqueuePendingList(NX_Thread *thread);
void NX_ThreadDequeuePendingList(NX_Thread *thread);
NX_Thread *NX_ThreadPickPendingList(void);

void NX_ThreadReadyRunLocked(NX_Thread *thread, int flags);
void NX_ThreadReadyRunUnlocked(NX_Thread *thread, int flags);

void NX_ThreadUnreadyRunLocked(NX_Thread *thread);
void NX_ThreadUnreadyRun(NX_Thread *thread);

void NX_ThreadExitProcess(NX_Thread *thread, NX_Process *process);

#endif /* __SCHED_THREAD__ */
