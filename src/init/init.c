/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Init OS 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-15     JasonHu           Init
 */

#include <xbook/init_call.h>
#include <sched/thread.h>
#define NX_LOG_NAME "InitCall"
#include <utils/log.h>
#include <xbook/debug.h>

NX_IMPORT NX_InitCallHandler __NX_InitCallStart[];
NX_IMPORT NX_InitCallHandler __NX_InitCallEnd[];
NX_IMPORT NX_InitCallHandler __NX_ExitCallStart[];
NX_IMPORT NX_InitCallHandler __NX_ExitCallEnd[];

void NX_CallInvoke(NX_InitCallHandler start[], NX_InitCallHandler end[])
{
	NX_InitCallHandler *func =  &(*start);
	for (;func < &(*end); func++)
    {
		(*func)();
    }
}

void NX_InitCallInvoke(void)
{
    NX_CallInvoke(__NX_InitCallStart, __NX_InitCallEnd);
}

void NX_ExitCallInvoke(void)
{
    NX_CallInvoke(__NX_ExitCallStart, __NX_ExitCallEnd);
}

NX_INTERFACE NX_WEAK_SYM void HAL_PlatformMain(void)
{
    NX_LOG_I("Deafult platform main running...\n");
}

NX_PRIVATE void CallsEntry(void *arg)
{
    NX_InitCallInvoke();
    HAL_PlatformMain();
}

void NX_CallsInit(void)
{
    NX_Thread *thread = NX_ThreadCreate("Calls", CallsEntry, NX_NULL);
    NX_ASSERT(thread != NX_NULL);
    NX_ASSERT(NX_ThreadRun(thread) == NX_EOK);
}
