/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: user call system function
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-31      JasonHu           Init
 */

#include <base/syscall.h>
#include <base/thread.h>
#define NX_LOG_NAME "syscall"
#include <base/log.h>
#include <base/process.h>
#include <base/debug.h>
#include <base/thread.h>
#include <base/vfs.h>
#include <base/hub.h>
#include <base/vmspace.h>
#include <base/uaccess.h>
#include <base/page.h>
#include <base/string.h>
#include <base/snapshot.h>
#include <base/time.h>
#include <base/malloc.h>
#include <base/driver.h>
#include <base/signal.h>

#include "process_impl.h"

NX_PRIVATE int SysInvalidCall(void)
{
    NX_Thread *cur = NX_ThreadSelf();
    NX_LOG_E("thread %s/%d call invalid syscall!", cur->name, cur->tid);    
    return 0;
}

NX_PRIVATE int SysDebugLog(const char *buf, int size)
{
    /* FIXME: check buf -> buf + size accessable */
    NX_Printf(buf);
    return 0;
}

NX_PRIVATE void SysProcessExit(NX_U32 exitCode)
{
    NX_ProcessExit(exitCode);
    NX_PANIC("process exit syscall failed !");
}

NX_PRIVATE NX_Error SysProcessLaunch(char *path, NX_U32 flags, NX_U32 *outExitCode, char *cmd, char *env)
{
    NX_U32 exitCode = 0;
    NX_Error err = NX_EOK;
    err = NX_ProcessLaunch(path, flags, &exitCode, cmd, env);
    if (outExitCode)
    {
        NX_CopyToUser((char *)outExitCode, (char *)&exitCode, sizeof(exitCode));
    }
    return err;
}

NX_PRIVATE NX_Error SysVfsMount(const char * dev, const char * dir, const char * fsname, NX_U32 flags)
{
    return NX_VfsMountFileSystem(dev, dir, fsname, flags);
}

NX_PRIVATE NX_Error SysVfsUnmount(const char * path)
{
    return NX_VfsUnmountFileSystem(path);
}

NX_PRIVATE NX_Error SysVfsSync(void)
{
    return NX_VfsSync();
}

NX_PRIVATE int SysVfsOpen(const char * path, NX_U32 flags, NX_U32 mode, NX_Error *outErr)
{
    return NX_VfsOpen(path, flags, mode, outErr);
}

NX_PRIVATE NX_Error SysVfsClose(int fd)
{
    return NX_VfsClose(fd);
}

/* NOTICE: To compate 32 bit cpu, syscall need use NX_Size not NX_U64 */
NX_PRIVATE NX_Size SysVfsRead(int fd, void * buf, NX_Size len, NX_Error *outErr)
{
    return NX_VfsRead(fd, buf, len, outErr);
}

/* NOTICE: To compate 32 bit cpu, syscall need use NX_Size not NX_U64 */
NX_PRIVATE NX_Size SysVfsWrite(int fd, void * buf, NX_Size len, NX_Error *outErr)
{
    return NX_VfsWrite(fd, buf, len, outErr);
}

NX_Error SysVfsIoctl(int fd, NX_U32 cmd, void *arg)
{
    return NX_VfsIoctl(fd, cmd, arg);
}

/* NOTICE: To compate 32 bit cpu, syscall need use NX_Offset not NX_I64 */
NX_PRIVATE NX_Offset SysVfsFileSeek(int fd, NX_Offset off, int whence, NX_Error *outErr)
{
    return NX_VfsFileSeek(fd, off, whence, outErr);
}

NX_PRIVATE NX_Error SysVfsFileSync(int fd)
{
    return NX_VfsFileSync(fd);
}

NX_PRIVATE NX_Error SysVfsFileChmod(int fd, NX_U32 mode)
{
    return NX_VfsFileChmod(fd, mode);
}

NX_PRIVATE NX_Error SysVfsFileStat(int fd, NX_VfsStatInfo * st)
{
    return NX_VfsFileStat(fd, st);
}

NX_PRIVATE int SysVfsOpenDir(const char * name, NX_Error *outErr)
{
    return NX_VfsOpenDir(name, outErr);
}

NX_PRIVATE NX_Error SysVfsCloseDir(int fd)
{
    return NX_VfsCloseDir(fd);
}
    
NX_PRIVATE NX_Error SysVfsReadDir(int fd, NX_VfsDirent * dir)
{
    return NX_VfsReadDir(fd, dir);
}

NX_PRIVATE NX_Error SysVfsRewindDir(int fd)
{
    return NX_VfsRewindDir(fd);
}

NX_PRIVATE NX_Error SysVfsMakeDir(const char * path, NX_U32 mode)
{
    return NX_VfsMakeDir(path, mode);
}

NX_PRIVATE NX_Error SysVfsRemoveDir(const char * path)
{
    return NX_VfsRemoveDir(path);
}

NX_PRIVATE NX_Error SysVfsRename(const char * src, const char * dst)
{
    return NX_VfsRename(src, dst);
}

NX_PRIVATE NX_Error SysVfsUnlink(const char * path)
{
    return NX_VfsUnlink(path);
}

NX_PRIVATE NX_Error SysVfsAccess(const char * path, NX_U32 mode)
{
    return NX_VfsAccess(path, mode);
}

NX_PRIVATE NX_Error SysVfsChmod(const char * path, NX_U32 mode)
{
    return NX_VfsChmod(path, mode);
}

NX_PRIVATE NX_Error SysVfsStat(const char * path, NX_VfsStatInfo * st)
{
    return NX_VfsStat(path, st);
}

NX_PRIVATE NX_Error SysHubRegister(const char *name, NX_Size maxClient)
{
    return NX_HubRegister(name, maxClient, NX_NULL);
}

NX_PRIVATE NX_Error SysHubUnregister(const char *name)
{
    return NX_HubUnregister(name);
}

NX_PRIVATE NX_Error SysHubCallParam(NX_Hub *hub, NX_HubParam *param, NX_Size *retVal)
{
    return NX_HubCallParam(hub, param, retVal);
}

NX_PRIVATE NX_Error SysHubCallParamName(const char *name, NX_HubParam *param, NX_Size *retVal)
{
    return NX_HubCallParamName(name, param, retVal);
}

NX_PRIVATE NX_Error SysHubReturn(NX_Size retVal, NX_Error retErr)
{
    return NX_HubReturn(retVal, retErr);
}

NX_PRIVATE NX_Error SysHubPoll(NX_HubParam *param)
{
    return NX_HubPoll(param);
}

NX_PRIVATE void *SysHubTranslate(void *addr, NX_Size size)
{
    return NX_HubTranslate(addr, size);
}

NX_PRIVATE void *SysMemMap(void * addr, NX_Size length, NX_U32 prot, NX_Error *outErr)
{
    NX_Error err;
    NX_Thread *self;
    void *outAddr = NX_NULL;
    NX_UArch attr;

    if (!length || !prot)
    {
        err = NX_EINVAL;
        if (outErr)
        {
            NX_CopyToUser((char *)outErr, (char *)&err, sizeof(NX_Error));
        }
        return NX_NULL;
    }

    self = NX_ThreadSelf();

    /* make attr */
    attr = NX_PAGE_ATTR_USER & (~NX_PAGE_ATTR_RWX);
    if (prot & NX_PROT_READ)
    {
        attr |= NX_PAGE_ATTR_READ;
    }
    if (prot & NX_PROT_WRITE)
    {
        attr |= NX_PAGE_ATTR_WRITE;
    }
    if (prot & NX_PROT_EXEC)
    {
        attr |= NX_PAGE_ATTR_EXEC;
    }

    err = NX_VmspaceMap(&self->resource.process->vmspace,
                        (NX_Addr)addr, length, attr, 0, &outAddr);

    if (outErr)
    {
        NX_CopyToUser((char *)outErr, (char *)&err, sizeof(NX_Error));
    }
    return outAddr;
}

NX_PRIVATE NX_Error SysMemUnmap(void *addr, NX_Size length)
{
    NX_Thread *self;
    
    if (addr == NX_NULL || !length)
    {
        return NX_EINVAL;
    }

    self = NX_ThreadSelf();

    return NX_VmspaceUnmap(&self->resource.process->vmspace, NX_PAGE_ALIGNDOWN((NX_Addr)addr), length);
}

NX_PRIVATE void *SysMemHeap(void *addr, NX_Error *outErr)
{
    NX_Error err;
    NX_Thread *self;
    void *heapAddr;

    self = NX_ThreadSelf();

    heapAddr = NX_VmspaceUpdateHeap(&self->resource.process->vmspace, (NX_Addr)addr, &err);
    if (outErr)
    {
        NX_CopyToUser((char *)outErr, (char *)&err, sizeof(NX_Error));
    }
    return heapAddr;
}

NX_PRIVATE NX_Error SysProcessGetCwd(char * buf, NX_Size length)
{
    char * cwd;

    if (!buf || !length)
    {
        return NX_EINVAL;
    }

    cwd = NX_ProcessGetCwd(NX_ThreadSelf()->resource.process);
    if (!cwd)
    {
        return NX_ENORES;
    }

    NX_CopyToUser(buf, cwd, NX_MIN((int)length, NX_StrLen(cwd)));
    return NX_EOK;
}

NX_PRIVATE NX_Error SysProcessSetCwd(char * buf)
{
    char * cwd;
    NX_Process * process;

    if (!buf)
    {
        return NX_EINVAL;
    }

    process = NX_ThreadSelf()->resource.process;

    cwd = NX_ProcessGetCwd(process);
    if (!cwd)
    {
        return NX_ENORES;
    }

    return NX_ProcessSetCwd(process, buf);
}

NX_PRIVATE NX_Error SysSoltClose(NX_Solt solt)
{
    return NX_ProcessUninstallSolt(NX_ProcessCurrent(), solt);
}

NX_PRIVATE NX_Solt SysSnapshotCreate(NX_U32 snapshotType, NX_U32 flags, NX_Error * outErr)
{
    return NX_SnapshotCreate(snapshotType, flags, outErr);
}

NX_PRIVATE NX_Error SysSnapshotFirst(NX_Solt solt, void * object)
{
    return NX_SnapshotFirst(solt, object);
}

NX_PRIVATE NX_Error SysSnapshotNext(NX_Solt solt, void * object)
{
    return NX_SnapshotNext(solt, object);
}

NX_PRIVATE NX_Error SysThreadSleep(NX_UArch microseconds)
{
    return NX_ThreadSleep(microseconds);
}

NX_PRIVATE NX_TimeVal SysClockGetMillisecond(void)
{
    return NX_ClockTickGetMillisecond();
}

NX_PRIVATE NX_Error SysTimeSet(NX_Time * time)
{
    return NX_TimeSet(time);
}

NX_PRIVATE NX_Error SysTimeGet(NX_Time * time)
{
    return NX_TimeGet(time);
}

NX_PRIVATE void UserThreadEntry(void * arg)
{
    NX_Size userStackTop;
    NX_Thread * self = NX_ThreadSelf();
    
    userStackTop = NX_ALIGN_DOWN((NX_Size)(self->userStackBase + self->userStackSize), 8);
    
    NX_ProcessExecuteUserThread((void *)self->userHandler, (void *)userStackTop, (void *)(self->stackBase + self->stackSize), arg);
    NX_PANIC("user thread exit!");
}

NX_PRIVATE NX_Error SysThreadCreate(NX_ThreadAttr * attr, NX_ThreadHandler handler, void * arg, NX_U32 flags, NX_Solt * outSolt)
{
    NX_ThreadAttr threadAttr;
    void * stackBase = NX_NULL;
    NX_Process * process;
    NX_Error err;
    NX_Thread * thread;
    NX_Solt solt;

    if (!attr || !handler)
    {
        return NX_EINVAL;
    }

    NX_CopyFromUser((char *)&threadAttr, (char *)attr, sizeof(threadAttr));

    if (!threadAttr.stackSize)
    {
        return NX_EINVAL;
    }

    /* check priority */
    if (threadAttr.schedPriority < NX_THREAD_PRIORITY_LOW)
    {
        threadAttr.schedPriority = NX_THREAD_PRIORITY_LOW;
    }

    if (threadAttr.schedPriority > NX_THREAD_PRIORITY_HIGH)
    {
        threadAttr.schedPriority = NX_THREAD_PRIORITY_HIGH;
    }

    process = NX_ProcessCurrent();
    if (!process)
    {
        NX_LOG_E("create thread no process!");
        return NX_EFAULT;
    }

    /* map stack */
    err = NX_VmspaceMap(&process->vmspace, 0, threadAttr.stackSize, NX_PAGE_ATTR_USER, 0, &stackBase);
    if (err != NX_EOK)
    {
        NX_LOG_E("map user stack error!");
        return err;
    }
    NX_ASSERT(stackBase);

    /* create thread */
    thread = NX_ThreadCreate("uthread", UserThreadEntry, arg, threadAttr.schedPriority);
    if (thread == NX_NULL)
    {
        NX_LOG_E("create thread error!");
        NX_VmspaceUnmap(&process->vmspace, (NX_Addr)stackBase, threadAttr.stackSize);
        return NX_ENORES;
    }

    thread->userHandler = handler;
    thread->userStackSize = threadAttr.stackSize;
    thread->userStackBase = stackBase;

    /* map tls */
    if (NX_ProcessMapTls(process, thread) != NX_EOK)
    {
        NX_LOG_E("map thread tls error!");
        NX_ThreadDestroy(thread);
        NX_VmspaceUnmap(&process->vmspace, (NX_Addr)stackBase, threadAttr.stackSize);
        return NX_ENOMEM;
    }

    /* install thread solt */
    solt = NX_SOLT_INVALID_VALUE;
    if (NX_ProcessInstallSolt(process, thread, NX_EXOBJ_THREAD, NX_NULL, &solt) != NX_EOK)
    {
        NX_LOG_E("install thread object failed!");
        NX_ProcessUnmapTls(process, thread);
        NX_ThreadDestroy(thread);
        NX_VmspaceUnmap(&process->vmspace, (NX_Addr)stackBase, threadAttr.stackSize);
        return NX_ENORES;
    }

    if (outSolt)
    {
        NX_CopyToUser((char *)outSolt, (char *)&solt, sizeof(solt));
    }

    /* process attach thread */
    NX_ProcessAppendThread(process, thread);

    /* start thread */
    NX_ThreadStart(thread);
    
    if (flags & NX_THREAD_CREATE_SUSPEND)
    {
        NX_ThreadBlock(thread);
    }

    return NX_EOK;
}

NX_PRIVATE void SysThreadExit(NX_U32 exitCode)
{
    NX_ThreadExit(exitCode);
}

NX_PRIVATE NX_Error SysThreadSuspend(NX_Solt solt)
{
    NX_ExposedObject * exobj;

    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    exobj = NX_ProcessGetSolt(NX_ProcessCurrent(), solt);
    if (exobj == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_THREAD)
    {
        return NX_ENORES;
    }
    return NX_ThreadBlock(exobj->object);
}

NX_PRIVATE NX_Error SysThreadResume(NX_Solt solt)
{
    NX_ExposedObject * exobj;

    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    exobj = NX_ProcessGetSolt(NX_ProcessCurrent(), solt);
    if (exobj == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_THREAD)
    {
        return NX_ENORES;
    }
    return NX_ThreadUnblock(exobj->object);
}

NX_PRIVATE NX_Error SysThreadWait(NX_Solt solt, NX_U32 * outExitCode)
{
    NX_ExposedObject * exobj;
    NX_Error err;
    NX_U32 exitCode;

    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    exobj = NX_ProcessGetSolt(NX_ProcessCurrent(), solt);
    if (exobj == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_THREAD)
    {
        return NX_ENORES;
    }

    err = NX_ThreadWait(exobj->object, &exitCode);
    if (err != NX_EOK)
    {
        return err;
    }

    if (outExitCode)
    {
        NX_CopyToUser((char *)outExitCode, (char *)&exitCode, sizeof(exitCode));
    }
    
    return NX_EOK;
}

NX_PRIVATE NX_Error SysThreadTerminate(NX_Solt solt, NX_U32 exitCode)
{
    NX_ExposedObject * exobj;
    NX_Error err;

    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    exobj = NX_ProcessGetSolt(NX_ProcessCurrent(), solt);
    if (exobj == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_THREAD)
    {
        return NX_ENORES;
    }

    err = NX_ThreadTerminate(exobj->object, exitCode);
    if (err != NX_EOK)
    {
        return err;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error SysThreadGetId(NX_Solt solt, NX_U32 * outId)
{
    NX_ExposedObject * exobj;
    NX_Thread * thread;

    if (solt == NX_SOLT_INVALID_VALUE || !outId)
    {
        return NX_EINVAL;
    }

    exobj = NX_ProcessGetSolt(NX_ProcessCurrent(), solt);
    if (exobj == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_THREAD)
    {
        return NX_ENORES;
    }

    thread = (NX_Thread *)exobj->object;

    NX_CopyToUser((char *)outId, (char *)&thread->tid, sizeof(thread->tid));

    return NX_EOK;
}

NX_PRIVATE NX_U32 SysThreadGetCurrentId(void)
{
    NX_Thread * self;

    self = NX_ThreadSelf();
    return (NX_U32)self->tid;
}

NX_PRIVATE NX_Error SysThreadGetCurrent(NX_Solt * outSolt)
{
    NX_Solt solt;
    NX_Process * process;
    NX_Thread * self;
    
    if (!outSolt)
    {
        return NX_EINVAL;
    }

    self = NX_ThreadSelf();
    process = NX_ProcessCurrent();
    if (!process)
    {
        return NX_EFAULT;
    }

    solt = NX_ProcessLocateSolt(process, self, NX_EXOBJ_THREAD);
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_ENORES;
    }

    NX_CopyToUser((char *)outSolt, (char *)&solt, sizeof(solt));

    return NX_EOK;
}

NX_PRIVATE NX_Error SysThreadGetProcessId(NX_Solt solt, NX_U32 * outId)
{
    NX_ExposedObject * exobj;
    NX_Thread * thread;
    NX_Process * process;

    if (solt == NX_SOLT_INVALID_VALUE || !outId)
    {
        return NX_EINVAL;
    }

    exobj = NX_ProcessGetSolt(NX_ProcessCurrent(), solt);
    if (exobj == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_THREAD)
    {
        return NX_ENORES;
    }

    thread = (NX_Thread *)exobj->object;
    
    process = thread->resource.process;

    NX_CopyToUser((char *)outId, (char *)&process->pid, sizeof(process->pid));

    return NX_EOK;
}

NX_PRIVATE NX_Error SemaphoreCloseSolt(void * object, NX_ExposedObjectType type)
{
    NX_Semaphore * sem;
    NX_Error err;

    if (type != NX_EXOBJ_SEMAPHORE)
    {
        return NX_ENORES;
    }

    sem = (NX_Semaphore *) object;
    NX_ASSERT(sem);

    if ((err = NX_SemaphoreState(sem)) != NX_EOK)
    {
        return err;
    }

    NX_MemZero(sem, sizeof(sem));
    NX_MemFree(sem);

    return NX_EOK;
}

NX_PRIVATE NX_Error SysSemaphoreCreate(NX_IArch value, NX_Solt * outSolt)
{
    NX_Semaphore * sem;
    NX_Error err;
    NX_Solt solt = NX_SOLT_INVALID_VALUE;
    NX_Process * process;
    
    if (!outSolt)
    {
        return NX_EINVAL;
    }

    sem = NX_MemAlloc(sizeof(NX_Semaphore));
    if (sem == NX_NULL)
    {
        return NX_ENOMEM;
    }

    if ((err = NX_SemaphoreInit(sem, value)) != NX_EOK)
    {
        NX_MemFree(sem);
        return err;
    }

    process = NX_ProcessCurrent();
    if ((err = NX_ProcessInstallSolt(process, sem, NX_EXOBJ_SEMAPHORE, SemaphoreCloseSolt, &solt)) != NX_EOK)
    {
        NX_MemFree(sem);
        return err;
    }

    NX_CopyToUser((char *)outSolt, (char *)&solt, sizeof(solt));

    return NX_EOK;
}

NX_PRIVATE NX_Error SysSemaphoreDestroy(NX_Solt solt)
{
    NX_Semaphore * sem;
    NX_Error err;
    NX_Process * process;
    NX_ExposedObject * exobj;
    
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_SEMAPHORE)
    {
        return NX_ENORES;
    }

    sem = (NX_Semaphore *)exobj->object;
    NX_ASSERT(sem);

    if ((err = NX_SemaphoreState(sem)) != NX_EOK)
    {
        return err;
    }

    if ((err = NX_ProcessUninstallSolt(process, solt)) != NX_EOK)
    {
        return err;
    }

    NX_MemZero(sem, sizeof(sem));
    NX_MemFree(sem);

    return NX_EOK;
}

NX_PRIVATE NX_Error SysSemaphoreWait(NX_Solt solt)
{
    NX_Semaphore * sem;
    NX_Error err;
    NX_Process * process;
    NX_ExposedObject * exobj;
    
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_SEMAPHORE)
    {
        return NX_ENORES;
    }

    sem = (NX_Semaphore *)exobj->object;
    NX_ASSERT(sem);

    if ((err = NX_SemaphoreWait(sem)) != NX_EOK)
    {
        return err;
    }

    return NX_EOK;
}

NX_PRIVATE NX_Error SysSemaphoreTryWait(NX_Solt solt)
{
    NX_Semaphore * sem;
    NX_Error err;
    NX_Process * process;
    NX_ExposedObject * exobj;
    
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_SEMAPHORE)
    {
        return NX_ENORES;
    }

    sem = (NX_Semaphore *)exobj->object;
    NX_ASSERT(sem);

    if ((err = NX_SemaphoreTryWait(sem)) != NX_EOK)
    {
        return err;
    }

    return NX_EOK;
}

NX_PRIVATE NX_Error SysSemaphoreSignal(NX_Solt solt)
{
    NX_Semaphore * sem;
    NX_Error err;
    NX_Process * process;
    NX_ExposedObject * exobj;
    
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_SEMAPHORE)
    {
        return NX_ENORES;
    }

    sem = (NX_Semaphore *)exobj->object;
    NX_ASSERT(sem);

    if ((err = NX_SemaphoreSignal(sem)) != NX_EOK)
    {
        return err;
    }

    return NX_EOK;
}

NX_PRIVATE NX_Error SysSemaphoreSignalAll(NX_Solt solt)
{
    NX_Semaphore * sem;
    NX_Error err;
    NX_Process * process;
    NX_ExposedObject * exobj;
    
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_SEMAPHORE)
    {
        return NX_ENORES;
    }

    sem = (NX_Semaphore *)exobj->object;
    NX_ASSERT(sem);

    if ((err = NX_SemaphoreSignalAll(sem)) != NX_EOK)
    {
        return err;
    }

    return NX_EOK;
}

NX_PRIVATE NX_Error SysSemaphoreGetValue(NX_Solt solt, NX_IArch * outValue)
{
    NX_Semaphore * sem;
    NX_Process * process;
    NX_ExposedObject * exobj;
    NX_IArch value;
    
    if (solt == NX_SOLT_INVALID_VALUE || !outValue)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_SEMAPHORE)
    {
        return NX_ENORES;
    }

    sem = (NX_Semaphore *)exobj->object;
    NX_ASSERT(sem);

    value = NX_SemaphoreGetValue(sem);

    NX_CopyToUser((char *)outValue, (char *)&value, sizeof(value));

    return NX_EOK;
}

NX_PRIVATE NX_Error MutexCloseSolt(void * object, NX_ExposedObjectType type)
{
    NX_Mutex * mutex;
    NX_Error err;

    if (type != NX_EXOBJ_MUTEX)
    {
        return NX_ENORES;
    }

    mutex = (NX_Mutex *) object;
    NX_ASSERT(mutex);

    if ((err = NX_MutexState(mutex)) != NX_EOK)
    {
        return err;
    }

    NX_MemZero(mutex, sizeof(mutex));
    NX_MemFree(mutex);

    return NX_EOK;
}

#define NX_MUTEX_ATTR_LOCKED 0x01

NX_PRIVATE NX_Error SysMutexCreate(NX_U32 attr, NX_Solt * outSolt)
{
    NX_Mutex * mutex;
    NX_Error err;
    NX_Solt solt = NX_SOLT_INVALID_VALUE;
    NX_Process * process;
    
    if (!outSolt)
    {
        return NX_EINVAL;
    }

    mutex = NX_MemAlloc(sizeof(NX_Mutex));
    if (mutex == NX_NULL)
    {
        return NX_ENOMEM;
    }

    if (attr & NX_MUTEX_ATTR_LOCKED)
    {
        if ((err = NX_MutexInitLocked(mutex)) != NX_EOK)
        {
            NX_MemFree(mutex);
            return err;
        }
    }
    else
    {
        if ((err = NX_MutexInit(mutex)) != NX_EOK)
        {
            NX_MemFree(mutex);
            return err;
        }
    }

    process = NX_ProcessCurrent();
    if ((err = NX_ProcessInstallSolt(process, mutex, NX_EXOBJ_MUTEX, MutexCloseSolt, &solt)) != NX_EOK)
    {
        NX_MemFree(mutex);
        return err;
    }

    NX_CopyToUser((char *)outSolt, (char *)&solt, sizeof(solt));

    return NX_EOK;
}

NX_PRIVATE NX_Error SysMutexDestroy(NX_Solt solt)
{
    NX_Mutex * mutex;
    NX_Error err;
    NX_Process * process;
    NX_ExposedObject * exobj;
    
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_MUTEX)
    {
        return NX_ENORES;
    }

    mutex = (NX_Mutex *)exobj->object;
    NX_ASSERT(mutex);

    if ((err = NX_MutexState(mutex)) != NX_EOK)
    {
        return err;
    }

    if ((err = NX_ProcessUninstallSolt(process, solt)) != NX_EOK)
    {
        return err;
    }

    NX_MemZero(mutex, sizeof(mutex));
    NX_MemFree(mutex);

    return NX_EOK;
}

NX_PRIVATE NX_Error SysMutexAcquire(NX_Solt solt)
{
    NX_Mutex * mutex;
    NX_Error err;
    NX_Process * process;
    NX_ExposedObject * exobj;
    
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_MUTEX)
    {
        return NX_ENORES;
    }

    mutex = (NX_Mutex *)exobj->object;
    NX_ASSERT(mutex);

    if ((err = NX_MutexLock(mutex)) != NX_EOK)
    {
        return err;
    }

    return NX_EOK;
}

NX_PRIVATE NX_Error SysMutexTryAcquire(NX_Solt solt)
{
    NX_Mutex * mutex;
    NX_Error err;
    NX_Process * process;
    NX_ExposedObject * exobj;
    
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_MUTEX)
    {
        return NX_ENORES;
    }

    mutex = (NX_Mutex *)exobj->object;
    NX_ASSERT(mutex);

    if ((err = NX_MutexTryLock(mutex)) != NX_EOK)
    {
        return err;
    }

    return NX_EOK;
}

NX_PRIVATE NX_Error SysMutexRelease(NX_Solt solt)
{
    NX_Mutex * mutex;
    NX_Error err;
    NX_Process * process;
    NX_ExposedObject * exobj;
    
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_MUTEX)
    {
        return NX_ENORES;
    }

    mutex = (NX_Mutex *)exobj->object;
    NX_ASSERT(mutex);

    if ((err = NX_MutexUnlock(mutex)) != NX_EOK)
    {
        return err;
    }

    return NX_EOK;
}

NX_PRIVATE NX_Error SysMutexAcquirable(NX_Solt solt)
{
    NX_Mutex * mutex;
    NX_Error err;
    NX_Process * process;
    NX_ExposedObject * exobj;
    
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_MUTEX)
    {
        return NX_ENORES;
    }

    mutex = (NX_Mutex *)exobj->object;
    NX_ASSERT(mutex);

    if ((err = NX_MutexState(mutex)) != NX_EOK)
    {
        return err;
    }

    return NX_EOK;
}

NX_PRIVATE NX_Error DeviceCloseSolt(void * object, NX_ExposedObjectType type)
{
    NX_Device * dev;

    if (type != NX_EXOBJ_DEVICE)
    {
        return NX_ENORES;
    }

    dev = (NX_Device *) object;
    NX_ASSERT(dev);

    return NX_DeviceClose(dev);
}

NX_PRIVATE NX_Error SysDeviceOpen(const char * name, NX_U32 flags, NX_Solt * outSolt)
{
    NX_Device * dev = NX_NULL;
    NX_Error err;
    NX_Solt solt = NX_SOLT_INVALID_VALUE;
    NX_Process * process;
    
    if (!name || !outSolt)
    {
        return NX_EINVAL;
    }

    err = NX_DeviceOpen(name, flags, &dev);
    if (err != NX_EOK)
    {
        return err;
    }

    process = NX_ProcessCurrent();
    if ((err = NX_ProcessInstallSolt(process, dev, NX_EXOBJ_DEVICE, DeviceCloseSolt, &solt)) != NX_EOK)
    {
        NX_DeviceClose(dev);
        return err;
    }

    NX_CopyToUser((char *)outSolt, (char *)&solt, sizeof(solt));

    return NX_EOK;
}

NX_PRIVATE NX_Error SysDeviceRead(NX_Solt solt, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen)
{
    NX_Device * dev = NX_NULL;
    NX_Error err;
    NX_Process * process;
    NX_ExposedObject * exobj;
    NX_Size readLen = 0;
    
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_DEVICE)
    {
        return NX_ENORES;
    }

    dev = (NX_Device *)exobj->object;
    NX_ASSERT(dev);

    if ((err = NX_DeviceRead(dev, buf, off, len, &readLen)) != NX_EOK)
    {
        return err;
    }

    NX_CopyToUser((char *)outLen, (char *)&readLen, sizeof(readLen));

    return NX_EOK;
}

NX_PRIVATE NX_Error SysDeviceWrite(NX_Solt solt, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen)
{
    NX_Device * dev = NX_NULL;
    NX_Error err;
    NX_Process * process;
    NX_ExposedObject * exobj;
    NX_Size writeLen = 0;
    
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_DEVICE)
    {
        return NX_ENORES;
    }

    dev = (NX_Device *)exobj->object;
    NX_ASSERT(dev);

    if ((err = NX_DeviceWrite(dev, buf, off, len, &writeLen)) != NX_EOK)
    {
        return err;
    }

    NX_CopyToUser((char *)outLen, (char *)&writeLen, sizeof(writeLen));

    return NX_EOK;
}

NX_PRIVATE NX_Error SysDeviceControl(NX_Solt solt, NX_U32 cmd, void *arg)
{
    NX_Device * dev = NX_NULL;
    NX_Error err;
    NX_Process * process;
    NX_ExposedObject * exobj;
    
    if (solt == NX_SOLT_INVALID_VALUE)
    {
        return NX_EINVAL;
    }

    process = NX_ProcessCurrent();
    if ((exobj = NX_ProcessGetSolt(process, solt)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if (exobj->type != NX_EXOBJ_DEVICE)
    {
        return NX_ENORES;
    }

    dev = (NX_Device *)exobj->object;
    NX_ASSERT(dev);

    if ((err = NX_DeviceControl(dev, cmd, arg)) != NX_EOK)
    {
        return err;
    }

    return NX_EOK;
}

NX_Error SysSignalSend(NX_U32 tid, NX_Signal signal, void * signalValue)
{
    return NX_SignalSend(tid, signal, signalValue);
}

/* xbook env syscall table  */
NX_PRIVATE const NX_SyscallHandler NX_SyscallTable[] = 
{
    SysInvalidCall,         /* 0 */
    SysDebugLog,            /* 1 */
    SysProcessExit,
    SysProcessLaunch,
    SysVfsMount,
    SysVfsUnmount,          /* 5 */
    SysVfsSync,
    SysVfsOpen,
    SysVfsClose,
    SysVfsRead,
    SysVfsWrite,            /* 10 */
    SysVfsFileSeek,
    SysVfsFileSync,
    SysVfsFileChmod,
    SysVfsFileStat,
    SysVfsOpenDir,          /* 15 */
    SysVfsCloseDir,
    SysVfsReadDir,
    SysVfsRewindDir,
    SysVfsMakeDir,
    SysVfsRemoveDir,        /* 20 */
    SysVfsRename,
    SysVfsUnlink,
    SysVfsAccess,
    SysVfsChmod,
    SysVfsStat,             /* 25 */
    SysHubRegister,
    SysHubUnregister,
    SysHubCallParam,
    SysHubCallParamName,
    SysHubReturn,           /* 30 */
    SysHubPoll,
    SysHubTranslate,
    SysVfsIoctl,
    SysMemMap,
    SysMemUnmap,            /* 35 */
    SysMemHeap,
    SysProcessGetCwd,
    SysProcessSetCwd,
    SysSoltClose,
    SysSnapshotCreate,      /* 40 */
    SysSnapshotFirst,
    SysSnapshotNext,
    SysThreadSleep,
    SysClockGetMillisecond,
    SysTimeSet,             /* 45 */
    SysTimeGet,
    SysThreadCreate,
    SysThreadExit,
    SysThreadSuspend,
    SysThreadResume,        /* 50 */
    SysThreadWait,
    SysThreadTerminate,
    SysThreadGetId,
    SysThreadGetCurrentId,
    SysThreadGetCurrent,    /* 55 */
    SysThreadGetProcessId,
    SysSemaphoreCreate,
    SysSemaphoreDestroy,
    SysSemaphoreWait,
    SysSemaphoreTryWait,    /* 60 */
    SysSemaphoreSignal,
    SysSemaphoreSignalAll,
    SysSemaphoreGetValue,
    SysMutexCreate,
    SysMutexDestroy,        /* 65 */
    SysMutexAcquire,
    SysMutexTryAcquire,
    SysMutexRelease,
    SysMutexAcquirable,
    SysDeviceOpen,          /* 70 */
    SysDeviceRead,
    SysDeviceWrite,
    SysDeviceControl,
    SysSignalSend,
};

/* posix env syscall table */
NX_PRIVATE const NX_SyscallHandler NX_SyscallTablePosix[] = 
{
    SysInvalidCall,    /* 0 */
};

NX_PRIVATE const NX_SyscallHandler NX_SyscallTableWin32[] = 
{
    SysInvalidCall,    /* 0 */
};

NX_SyscallHandler NX_SyscallGetHandler(NX_SyscallApi api)
{
    NX_SyscallHandler handler = SysInvalidCall;

    NX_U32 callNumber = NX_SYSCALL_NUMBER(api);
    NX_U32 callEnv = NX_SYSCALL_ENV(api);
    
    switch (callEnv)
    {
    case NX_SYSCALL_ENV_XBOOK:
        if (callNumber < NX_ARRAY_SIZE(NX_SyscallTable))
        {
            handler = NX_SyscallTable[callNumber];
        }
        break;
    
    case NX_SYSCALL_ENV_POSIX:
        if (callNumber < NX_ARRAY_SIZE(NX_SyscallTablePosix))
        {
            handler = NX_SyscallTablePosix[callNumber];
        }
        break;

    case NX_SYSCALL_ENV_WIN32:
        if (callNumber < NX_ARRAY_SIZE(NX_SyscallTableWin32))
        {
            handler = NX_SyscallTableWin32[callNumber];
        }
        break;

    default:
        break;
    }

    return handler;
}
