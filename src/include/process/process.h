/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Process
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-7       JasonHu           Init
 */

#ifndef __PROCESS_PROCESS___
#define __PROCESS_PROCESS___

#include <xbook.h>
#include <utils/list.h>
#include <sched/spin.h>
#include <mm/vmspace.h>
#include <fs/vfs.h>
#include <sched/semaphore.h>

#define NX_PROCESS_USER_SATCK_SIZE (NX_PAGE_SIZE * 4)

#define NX_PROC_FLAG_WAIT_START 0x01

struct NX_Process
{
    NX_U32 flags;
    NX_Vmspace vmspace;

    NX_Atomic threadCount;  /* thread count in this process */
    NX_List threadPoolListHead;    /* all thread on this process */

    NX_Spin lock;   /* lock for process */

    int exitCode;   /* exit code for process */
    int waitExitCode;   /* exit code for this process wait another process */

    NX_VfsFileTable *fileTable; /* file table */
    
    NX_Semaphore waiterSem; /* The semaphore of the process waiting for this process to exit */
    NX_Atomic waiterNumber; /* waiters */

    NX_I32 pid; /* process id */

    /* thread group */
};
typedef struct NX_Process NX_Process;

struct NX_ProcessOps
{
    NX_Error (*initUserSpace)(NX_Process *process, NX_Addr virStart, NX_Size size);
    NX_Error (*switchPageTable)(void *pageTable);
    void *(*getKernelPageTable)(void);
    void (*executeUser)(const void *text, void *userStack, void *kernelStack, void *args);
    NX_Error (*freePageTable)(NX_Vmspace *vmspace);
};

NX_INTERFACE NX_IMPORT struct NX_ProcessOps NX_ProcessOpsInterface; 

#define NX_ProcessInitUserSpace         NX_ProcessOpsInterface.initUserSpace
#define NX_ProcessSwitchPageTable       NX_ProcessOpsInterface.switchPageTable
#define NX_ProcessGetKernelPageTable    NX_ProcessOpsInterface.getKernelPageTable
#define NX_ProcessExecuteUser           NX_ProcessOpsInterface.executeUser
#define NX_ProcessFreePageTable         NX_ProcessOpsInterface.freePageTable

NX_Error NX_ProcessLaunch(char *name, char *path, NX_U32 flags, int *outPid);
void NX_ProcessExit(int exitCode);
NX_Error NX_ProcessWait(int pid, int *retCode);

#endif /* __PROCESS_PROCESS___ */
