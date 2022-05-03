/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: kernel snapshot
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-05-02     JasonHu           Init
 */

#ifndef __KERNEL_SNAPSHOT_H__
#define __KERNEL_SNAPSHOT_H__

#include <sched/thread.h>

#define NX_SNAPSHOT_PROCESS 1
#define NX_SNAPSHOT_THREAD  2
#define NX_SNAPSHOT_MAX     3

typedef struct NX_SnapshotThread
{
    NX_U32 usage;
    NX_U32 state;
    NX_U32 threadId;
    NX_U32 ownerProcessId;
    NX_U32 fixedPriority;
    NX_U32 priority;
    NX_U32 onCore;        /* thread on which core */
    NX_U32 coreAffinity;  /* thread would like to run on the core */
    NX_U32 flags;
    char name[NX_THREAD_NAME_LEN];
} NX_SnapshotThread;

typedef struct NX_SnapshotProcess
{
    NX_U32 usage;
    NX_U32 processId;
    NX_U32 threadCount;
    NX_U32 parentProcessId; 
    NX_U32 flags;
    char exePath[NX_VFS_MAX_PATH]; /* execute path */
} NX_SnapshotProcess;

typedef struct NX_SnapshotHead
{
    NX_List list;
    union 
    {
        NX_SnapshotThread thread;
        NX_SnapshotProcess process;
    } body;
} NX_SnapshotHead;

typedef struct NX_Snapshot
{
    NX_List listHead;
    NX_U32 snapshotType;
    NX_SnapshotHead * lastSnapshot;
} NX_Snapshot;

NX_Solt NX_SnapshotCreate(NX_U32 snapshotType, NX_U32 flags, NX_Error * outErr);

NX_Error NX_SnapshotFirst(NX_Solt solt, void * object);
NX_Error NX_SnapshotNext(NX_Solt solt, void * object);

#endif /* __KERNEL_SNAPSHOT_H__ */
