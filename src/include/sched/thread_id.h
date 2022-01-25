/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Thread ID management
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-7      JasonHu           Init
 */

#ifndef __SCHED_THREAD_ID__
#define __SCHED_THREAD_ID__

#include <xbook.h>
#include <sched/mutex.h>

#ifdef CONFIG_NX_MAX_THREAD_NR
#define NX_MAX_THREAD_NR CONFIG_NX_MAX_THREAD_NR
#else
#define NX_MAX_THREAD_NR 64
#endif

struct NX_ThreadID
{
    NX_U32 *maps;
    NX_U32 nextID;
    NX_Mutex idLock;
};

NX_PUBLIC int NX_ThreadIdAlloc(void);
NX_PUBLIC void NX_ThreadIdFree(int id);
NX_PUBLIC void NX_ThreadsInitID(void);

#endif /* __SCHED_THREAD_ID__ */
