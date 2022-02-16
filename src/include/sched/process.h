/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Process
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-7       JasonHu           Init
 */

#ifndef __SCHED_PROCESS___
#define __SCHED_PROCESS___

#include <xbook.h>
#include <utils/list.h>
#include <sched/spin.h>
#include <mm/vmspace.h>

struct NX_Process
{
    NX_U32 flags;
    NX_Vmspace vmspace;

    NX_Atomic threadCount;  /* thread count in this process */
    NX_List threadPoolListHead;    /* all thread on this process */

    NX_Spin lock;   /* lock for process */

    int exitCode;   /* exit code for process */

    /* User space memory manager */

    /* Hub Handle */

    /* thread group */
};
typedef struct NX_Process NX_Process;

struct NX_ProcessOps
{
    NX_Error (*initUserSpace)(NX_Process *process, NX_Addr virStart, NX_USize size);
    NX_Error (*switchPageTable)(void *pageTable);
    void *(*getKernelPageTable)(void);
    void (*executeUser)(const void *text, void *userStack, void *kernelStack, void *args);
};

NX_INTERFACE NX_IMPORT struct NX_ProcessOps NX_ProcessOpsInterface; 

#define NX_ProcessInitUserSpace         NX_ProcessOpsInterface.initUserSpace
#define NX_ProcessSwitchPageTable       NX_ProcessOpsInterface.switchPageTable
#define NX_ProcessGetKernelPageTable    NX_ProcessOpsInterface.getKernelPageTable
#define NX_ProcessExecuteUser           NX_ProcessOpsInterface.executeUser

NX_PUBLIC NX_Process *NX_ProcessCreate(NX_U32 flags);
NX_PUBLIC NX_Error NX_ProcessDestroy(NX_Process *process);
NX_PUBLIC NX_Error NX_ProcessExecute(char *name, char *path, NX_U32 flags);
NX_PUBLIC void NX_ProcessExit(int exitCode);

#endif /* __SCHED_PROCESS___ */
