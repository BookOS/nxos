/**
 * Copyright (c) 2018-2022, NXOS Development Team
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

#ifdef CONFIG_NX_ENABLE_PLATFORM_MAIN
NX_INTERFACE NX_WEAK_SYM void NX_HalPlatformMain(void)
{
    NX_LOG_I("Deafult platform main running...\n");
}
#endif

NX_PRIVATE void CallsEntry(void *arg)
{
    NX_InitCallInvoke();

#ifdef CONFIG_NX_ENABLE_MOUNT_TABLE
    {
        NX_Error err = NX_VfsMountFileSystem(CONFIG_NX_MOUNT_DEVICE_DEFAULT, CONFIG_NX_MOUNT_PATH_DEFAULT, CONFIG_NX_MOUNT_FSNAME_DEFAULT, 0);
        NX_LOG_I("mount dev:%s on path:%s as fs:%s with state %d", CONFIG_NX_MOUNT_DEVICE_DEFAULT, CONFIG_NX_MOUNT_PATH_DEFAULT, CONFIG_NX_MOUNT_FSNAME_DEFAULT, err);
    }
#endif /* CONFIG_NX_ENABLE_MOUNT_TABLE */

#ifdef CONFIG_NX_ENABLE_EXECUTE_USER
    {
        NX_Error err = NX_ProcessCreate(CONFIG_NX_FIRST_USER_NAME, CONFIG_NX_FIRST_USER_PATH, 0);
        NX_LOG_I("execute first user:%s on path:%s with state %d", CONFIG_NX_FIRST_USER_NAME, CONFIG_NX_FIRST_USER_PATH, err);        
    }
#endif /* CONFIG_NX_ENABLE_EXECUTE_USER */

#ifdef CONFIG_NX_ENABLE_PLATFORM_MAIN
    NX_HalPlatformMain();
#endif
}

void NX_CallsInit(void)
{
    NX_Thread *thread = NX_ThreadCreate("Calls", CallsEntry, NX_NULL, NX_THREAD_PRIORITY_HIGH);
    NX_ASSERT(thread != NX_NULL);
    NX_ASSERT(NX_ThreadStart(thread) == NX_EOK);
}
