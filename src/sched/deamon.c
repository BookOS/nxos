/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: deamon thread
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-2-18      JasonHu           Init
 */

#include <sched/thread.h>
#include <xbook/debug.h>

#define NX_LOG_NAME "deamon"
#include <utils/log.h>

/**
 * release resouce must for a thread run
 */
NX_PRIVATE void ThreadRelease(NX_Thread *thread)
{
    /* release thread with process */
    if (thread->resource.process != NX_NULL)
    {
        NX_ASSERT(NX_ProcessDestroy(thread->resource.process) == NX_EOK);
    }

    NX_ThreadDestroy(thread);
}

/**
 * system deamon thread for all cpus 
 */
NX_PRIVATE void DaemonThreadEntry(void *arg)
{
    NX_LOG_I("Daemon thread started.\n");

    NX_Thread *thread;
    while (1)
    {
        do
        {
            thread = NX_ThreadDeququeExitList();
            if (thread != NX_NULL)
            {
                NX_LOG_I("---> daemon release thread: %s/%d", thread->name, thread->tid);
                ThreadRelease(thread);
            }
        } while (thread != NX_NULL);
        
        NX_ThreadSleep(500);
    }
}

NX_PUBLIC void NX_ThreadInitDeamon(void)
{
    NX_Thread *deamonThread;
    /* init daemon thread */
    deamonThread = NX_ThreadCreate("daemon", DaemonThreadEntry, NX_NULL);
    NX_ASSERT(deamonThread != NX_NULL);
    NX_ASSERT(NX_ThreadRun(deamonThread) == NX_EOK);
}
