/**
 * Copyright (c) 2018-2021, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: NX_Thread ID test
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-16     JasonHu           Init
 */

#include <Utils/Demo.h>
#include <Utils/Log.h>
#include <Sched/Context.h>

#ifdef CONFIG_NX_DEMO_HAL_CONTEXT

#define STACK_SZ 2048

NX_U8 threadStack1[STACK_SZ];
NX_U8 threadStack2[STACK_SZ];
NX_U8 threadStack3[STACK_SZ];

NX_U8 *threadSP1;
NX_U8 *threadSP2;
NX_U8 *threadSP3;

NX_PRIVATE void thread_entry1(void *arg)
{
    NX_LOG_D("Thread 1... %x", arg);
    
    while (1)
    {
        NX_LOG_D("Hello 1");
        NX_ContextSwitchPrevNext((NX_UArch)&threadSP1, (NX_UArch)&threadSP2);
    }
}

NX_PRIVATE void thread_entry2(void *arg)
{
    NX_LOG_D("Thread 2... %x", arg);

    while (1)
    {
        NX_LOG_D("Hello 2");
        NX_ContextSwitchPrevNext((NX_UArch)&threadSP2, (NX_UArch)&threadSP3);
    }
}

NX_PRIVATE void thread_entry3(void *arg)
{
    NX_LOG_D("Thread 3... %x", arg);

    while (1)
    {
        NX_LOG_D("Hello 3");
        NX_ContextSwitchPrevNext((NX_UArch)&threadSP3, (NX_UArch)&threadSP1);
    }
}

NX_PUBLIC void Demo_HAL_Context(void)
{
    NX_LOG_I("start demo: HAL_Context");
    threadSP1 = NX_ContextInit(thread_entry1, NX_NULL, (void *) 0x12345678, threadStack1 + STACK_SZ);
    threadSP2 = NX_ContextInit(thread_entry2, NX_NULL, (void *) 0x12345678, threadStack2 + STACK_SZ);
    threadSP3 = NX_ContextInit(thread_entry3, NX_NULL, (void *) 0x12345678, threadStack3 + STACK_SZ);
    
    NX_LOG_I("demo: HAL_Context: switch");
    NX_ContextSwitchNext((NX_UArch)&threadSP1);
    NX_LOG_I("end demo: HAL_Context");
}

#endif
