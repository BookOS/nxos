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

#include <process/syscall.h>
#include <sched/thread.h>
#define NX_LOG_NAME "syscall"
#include <utils/log.h>
#include <process/process.h>
#include <xbook/debug.h>
#include <sched/thread.h>
#include <fs/vfs.h>
#include <ipc/hub.h>
#include <mm/vmspace.h>
#include <process/uaccess.h>
#include <mm/page.h>
#include <utils/string.h>
#include <process/snapshot.h>
#include <time/time.h>

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
    
    /* install thread solt */
    solt = NX_SOLT_INVALID_VALUE;
    if (NX_ProcessInstallSolt(process, thread, NX_EXOBJ_THREAD, NX_NULL, &solt))
    {
        NX_LOG_E("install thread object failed!");
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

NX_PRIVATE NX_Error NX_ThreadGetId(NX_Solt solt, NX_U32 * outId)
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

NX_PRIVATE NX_U32 NX_ThreadGetCurrentId(void)
{
    NX_Thread * self;

    self = NX_ThreadSelf();
    return (NX_U32)self->tid;
}

NX_PRIVATE NX_Error NX_ThreadGetCurrent(NX_Solt * outSolt)
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

NX_PRIVATE NX_Error NX_ThreadGetProcessId(NX_Solt solt, NX_U32 * outId)
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
    NX_ThreadGetId,
    NX_ThreadGetCurrentId,
    NX_ThreadGetCurrent,    /* 55 */
    NX_ThreadGetProcessId,
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
