/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: idle thread
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-2-18      JasonHu           Init
 */

#include <sched/thread.h>
#include <xbook/debug.h>

#define NX_LOG_NAME "idle"
#include <utils/log.h>
#include <utils/string.h>

/**
 * system idle thread on per cpu.
 */
NX_PRIVATE void IdleThreadEntry(void *arg)
{
    NX_LOG_I("Idle thread: %s startting...", NX_ThreadSelf()->name);
    int i = 0;
    while (1)
    {
        i++;
        NX_ThreadYield();
    }
}

void NX_ThreadInitIdle(void)
{
    NX_Thread *idleThread;
    int coreId;
    char name[8];

    /* init idle thread */
    for (coreId = 0; coreId < NX_MULTI_CORES_NR; coreId++)
    {
        NX_SNPrintf(name, 8, "idle%d", coreId);
        idleThread = NX_ThreadCreate(name, IdleThreadEntry, NX_NULL);
        NX_ASSERT(idleThread != NX_NULL);
        /* bind idle on each core */
        NX_ASSERT(NX_ThreadSetAffinity(idleThread, coreId) == NX_EOK);
        NX_ASSERT(NX_ThreadRun(idleThread) == NX_EOK);
    }
}
