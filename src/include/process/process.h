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
#include <xbook/exobj.h>

#define NX_PROCESS_USER_SATCK_SIZE (NX_PAGE_SIZE * 4)

#define NX_PROC_FLAG_NOWAIT 0x00
#define NX_PROC_FLAG_WAIT   0x01

#define NX_PROCESS_ARGS 2

#define NX_PROCESS_CWD_DEFAULT "/"

struct NX_Process
{
    NX_U32 flags;
    NX_Vmspace vmspace;

    NX_Atomic threadCount;  /* thread count in this process */
    NX_List threadPoolListHead;    /* all thread on this process */

    NX_Spin lock;   /* lock for process */

    NX_U32 exitCode;   /* exit code for process */
    NX_U32 waitExitCode;   /* exit code for this process wait another process */

    NX_VfsFileTable *fileTable; /* file table */
    
    NX_Semaphore waiterSem; /* The semaphore of the process waiting for this process to exit */

    NX_I32 pid; /* process id */
    NX_I32 parentPid; /* parent process id */

    void *args; /* process args */
    char cwd[NX_VFS_MAX_PATH]; /* current work diretory */
    char exePath[NX_VFS_MAX_PATH]; /* execute path */
    NX_ExposedObjectTable exobjTable;
};
typedef struct NX_Process NX_Process;

struct NX_ProcessOps
{
    NX_Error (*initUserSpace)(NX_Process *process, NX_Addr virStart, NX_Size size);
    NX_Error (*switchPageTable)(void *pageTable);
    void *(*getKernelPageTable)(void);
    void (*executeUser)(const void *text, void *userStack, void *kernelStack, void *args);
    void (*executeUserThread)(const void *text, void *userStack, void *kernelStack, void *arg);
    NX_Error (*freePageTable)(NX_Vmspace *vmspace);
};

NX_INTERFACE NX_IMPORT struct NX_ProcessOps NX_ProcessOpsInterface; 

#define NX_ProcessInitUserSpace         NX_ProcessOpsInterface.initUserSpace
#define NX_ProcessSwitchPageTable       NX_ProcessOpsInterface.switchPageTable
#define NX_ProcessGetKernelPageTable    NX_ProcessOpsInterface.getKernelPageTable
#define NX_ProcessExecuteUser           NX_ProcessOpsInterface.executeUser
#define NX_ProcessExecuteUserThread(text, userStack, kernelStack, arg) \
        NX_ProcessOpsInterface.executeUserThread(text, userStack, kernelStack, arg)
#define NX_ProcessFreePageTable         NX_ProcessOpsInterface.freePageTable

NX_Error NX_ProcessLaunch(char *path, NX_U32 flags, NX_U32 *exitCode, char *cmd, char *env);
void NX_ProcessExit(NX_U32 exitCode);

char * NX_ProcessGetCwd(NX_Process * process);
NX_Error NX_ProcessSetCwd(NX_Process * process, const char * path);

void NX_ProcessAppendThread(NX_Process *process, void *thread);

#define NX_ProcessGetSolt(process, solt) NX_ExposedObjectGet(&(process)->exobjTable, solt)
#define NX_ProcessLocateSolt(process, object, type) NX_ExposedObjectLocate(&(process)->exobjTable, object, type)
#define NX_ProcessInstallSolt(process, object, type, closeHandler, outSolt) NX_ExposedObjectInstall(&(process)->exobjTable,  object, type, closeHandler, outSolt)
#define NX_ProcessUninstallSolt(process, solt) NX_ExposedObjectUninstalll(&(process)->exobjTable, solt)
#define NX_ProcessCopySolt(dstProc, srcProc, solt) NX_ExposedObjectCopy(&(dstProc)->exobjTable, &(srcProc)->exobjTable, solt)

#define NX_ProcessCurrent() NX_ThreadSelf()->resource.process

#endif /* __PROCESS_PROCESS___ */
