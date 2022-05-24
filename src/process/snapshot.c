/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: kernel snapshot
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-05-03     JasonHu           Init
 */

#include <base/process.h>
#include <base/snapshot.h>
#include <base/malloc.h>
#include <base/memory.h>
#include <base/string.h>
#include <base/log.h>

NX_PRIVATE NX_SnapshotHead * CreateSnapshotHead(void)
{
    NX_SnapshotHead * head;

    head = NX_MemAllocEx(NX_SnapshotHead);
    if (!head)
    {
        return NX_NULL;
    }

    NX_MemZero(head, sizeof(NX_SnapshotHead));

    NX_ListInit(&head->list);
    return head;
}

NX_PRIVATE NX_Error SnapshotThreadWalkHandler(NX_Thread * thread, void * arg)
{
    NX_Snapshot * snapshot = (NX_Snapshot *)arg;
    NX_SnapshotHead * head;

    /* create thread snapshot */
    head = CreateSnapshotHead();
    if (!head)
    {
        return NX_ENOMEM;
    }

    /* fill struct */
    NX_StrCopyN(head->body.thread.name, thread->name, sizeof(head->body.thread.name));
    head->body.thread.usage = 0;
    head->body.thread.state = thread->state;
    head->body.thread.threadId = thread->tid;
    if (thread->resource.process)
    {
        head->body.thread.ownerProcessId = thread->resource.process->pid;
    }
    else
    {
        head->body.thread.ownerProcessId = 0;
    }
    head->body.thread.fixedPriority = thread->fixedPriority;
    head->body.thread.priority = thread->priority;
    head->body.thread.onCore = thread->onCore;
    head->body.thread.coreAffinity = thread->coreAffinity;
    head->body.thread.flags = 0;

    /* add to snapshot */
    NX_ListAddTail(&head->list, &snapshot->listHead);

    return NX_EOK;
}

NX_PRIVATE NX_Error SnapshotProcessWalkHandler(NX_Thread * thread, void * arg)
{
    NX_Snapshot * snapshot = (NX_Snapshot *)arg;
    NX_SnapshotHead * head;
    NX_Process * process;

    process = thread->resource.process;

    if (process == NX_NULL)
    {
        return NX_EOK;
    }

    /* create thread snapshot */
    head = CreateSnapshotHead();
    if (!head)
    {
        return NX_ENOMEM;
    }

    /* fill struct */
    NX_StrCopyN(head->body.process.exePath, process->exePath, sizeof(head->body.process.exePath));
    head->body.process.usage = 0;
    head->body.process.processId = process->pid;
    head->body.process.threadCount = NX_AtomicGet(&process->threadCount);
    head->body.process.parentProcessId = process->parentPid;
    head->body.process.flags = 0;

    /* add to snapshot */
    NX_ListAddTail(&head->list, &snapshot->listHead);

    return NX_EOK;
}

NX_PRIVATE NX_Error DoSnapshotThread(NX_Snapshot * snapshot, NX_U32 flags)
{
    NX_ThreadWalk(SnapshotThreadWalkHandler, snapshot);
    return NX_EOK;
}

NX_PRIVATE NX_Error DoSnapshotProcess(NX_Snapshot * snapshot, NX_U32 flags)
{
    NX_ThreadWalk(SnapshotProcessWalkHandler, snapshot);
    return NX_EOK;
}

NX_PRIVATE NX_Error DoSnapshot(NX_Snapshot * snapshot, NX_U32 snapshotType, NX_U32 flags)
{
    NX_Error err;

    switch (snapshotType)
    {
    case NX_SNAPSHOT_PROCESS:
        err = DoSnapshotProcess(snapshot, flags);
        break;
    case NX_SNAPSHOT_THREAD:
        err = DoSnapshotThread(snapshot, flags);
        break;
    default:
        err = NX_EINVAL;
        break;
    }
    return err;
}

NX_PRIVATE NX_Error __DestroySnapshot(NX_Snapshot * snapshot)
{
    NX_SnapshotHead * head, * next;

    NX_ListForEachEntrySafe(head, next, &snapshot->listHead, list)
    {
        NX_ListDelInit(&head->list);
        NX_MemFree(head);
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error DestroySnapshot(NX_Snapshot * snapshot)
{
    NX_Error err;

    switch (snapshot->snapshotType)
    {
    case NX_SNAPSHOT_PROCESS:
    case NX_SNAPSHOT_THREAD:
        err = __DestroySnapshot(snapshot);
        break;
    default:
        err = NX_EINVAL;
        break;
    }
    return err;
}

NX_PRIVATE NX_Error SnapshotClose(void * object, NX_ExposedObjectType type)
{
    NX_Snapshot * snapshot;

    if (object == NX_NULL || type != NX_EXOBJ_SNAPSHOT)
    {
        return NX_EPERM;
    }

    snapshot = (NX_Snapshot *)object;

    DestroySnapshot(snapshot);
    NX_MemFree(snapshot);

    return NX_EOK;
}

NX_Solt NX_SnapshotCreate(NX_U32 snapshotType, NX_U32 flags, NX_Error * outErr)
{
    NX_Process * process;
    NX_Snapshot * snapshot;
    NX_Solt solt = NX_SOLT_INVALID_VALUE;
    NX_Error err;

    if (!snapshotType || snapshotType >= NX_SNAPSHOT_MAX)
    {
        NX_ErrorSet(outErr, NX_EINVAL);
        return NX_SOLT_INVALID_VALUE;
    }

    process = NX_ProcessCurrent();

    if (!process)
    {
        NX_ErrorSet(outErr, NX_ENORES);
        return NX_SOLT_INVALID_VALUE;
    }

    /* alloc snapshot object */
    snapshot = NX_MemAlloc(sizeof(NX_Snapshot));
    if (!snapshot)
    {
        NX_ErrorSet(outErr, NX_ENOMEM);
        return NX_SOLT_INVALID_VALUE;
    }

    snapshot->snapshotType = snapshotType;
    NX_ListInit(&snapshot->listHead);
    snapshot->lastSnapshot = NX_NULL;

    /* do snapshot */
    if ((err = DoSnapshot(snapshot, snapshotType, flags)) != NX_EOK)
    {
        NX_MemFree(snapshot);
        NX_ErrorSet(outErr, err);
        return NX_SOLT_INVALID_VALUE;
    }

    /* install object */
    if ((err = NX_ProcessInstallSolt(process, snapshot, NX_EXOBJ_SNAPSHOT, SnapshotClose, &solt)) != NX_EOK)
    {
        DestroySnapshot(snapshot);
        NX_MemFree(snapshot);
        NX_ErrorSet(outErr, err);
        return NX_SOLT_INVALID_VALUE;
    }

    NX_ErrorSet(outErr, NX_EOK);
    return solt;
}

NX_Error NX_SnapshotFirst(NX_Solt solt, void * object)
{
    NX_Process * process;
    NX_SnapshotHead * head;
    NX_Snapshot * snapshot;

    if (solt == NX_SOLT_INVALID_VALUE || !object)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();

    if (process == NX_NULL)
    {
        return NX_ENORES;
    }

    NX_ExposedObject * exobj = NX_ProcessGetSolt(process, solt);
    if (exobj == NX_NULL)
    {
        return NX_EINVAL;
    }

    if (exobj->type != NX_EXOBJ_SNAPSHOT)
    {
        return NX_EPERM;
    }

    snapshot = (NX_Snapshot *)exobj->object;
    head = NX_ListFirstEntryOrNULL(&snapshot->listHead, NX_SnapshotHead, list);
    if (head == NX_NULL)
    {
        return NX_ENORES;
    }

    switch (snapshot->snapshotType)
    {
    case NX_SNAPSHOT_PROCESS:
        *(NX_SnapshotProcess *)object = head->body.process;
        break;
    case NX_SNAPSHOT_THREAD:
        *(NX_SnapshotThread *)object = head->body.thread;
        break;
    default:
        return NX_ENORES;
    }

    /* point to first list entry */
    snapshot->lastSnapshot = head;

    return NX_EOK;
}

NX_Error NX_SnapshotNext(NX_Solt solt, void * object)
{
    NX_Process * process;
    NX_SnapshotHead * head;
    NX_Snapshot * snapshot;

    if (solt == NX_SOLT_INVALID_VALUE || !object)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();

    if (process == NX_NULL)
    {
        return NX_ENORES;
    }

    NX_ExposedObject * exobj = NX_ProcessGetSolt(process, solt);
    if (exobj == NX_NULL)
    {
        return NX_EINVAL;
    }

    if (exobj->type != NX_EXOBJ_SNAPSHOT)
    {
        return NX_EPERM;
    }

    snapshot = (NX_Snapshot *)exobj->object;

    if (snapshot->lastSnapshot == NX_NULL)
    {
        return NX_EPERM;
    }

    if (NX_ListIsLast(&snapshot->lastSnapshot->list, &snapshot->listHead)) /* no object left */
    {
        return NX_ENORES;
    }

    head = NX_ListNextEntry(snapshot->lastSnapshot, list);

    switch (snapshot->snapshotType)
    {
    case NX_SNAPSHOT_PROCESS:
        *(NX_SnapshotProcess *)object = head->body.process;
        break;
    case NX_SNAPSHOT_THREAD:
        *(NX_SnapshotThread *)object = head->body.thread;
        break;
    default:
        return NX_ENORES;
    }

    /* point to next list */
    snapshot->lastSnapshot = head;

    return NX_EOK;
}
