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

#ifndef __SCHED_SEMAPHORE_H__
#define __SCHED_SEMAPHORE_H__

#include <xbook.h>
#include <sched/spin.h>
#include <utils/list.h>
#include <xbook/atomic.h>

typedef struct NX_Semaphore
{
    NX_Atomic value;
    NX_Spin lock;  /* lock for semaphore value */
    NX_List semWaitList;
    NX_U32 magic;  /* magic for semaphore init */
} NX_Semaphore;

NX_Error NX_SemaphoreInit(NX_Semaphore *sem, NX_IArch value);
NX_IArch NX_SemaphoreGetValue(NX_Semaphore *sem);
NX_Error NX_SemaphoreWait(NX_Semaphore *sem);
NX_Error NX_SemaphoreTryWait(NX_Semaphore *sem);
NX_Error NX_SemaphoreSignal(NX_Semaphore *sem);
NX_Error NX_SemaphoreSignalAll(NX_Semaphore *sem);
NX_Error NX_SemaphoreState(NX_Semaphore *sem);

#endif /* __SCHED_SEMAPHORE_H__ */
