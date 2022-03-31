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

NX_PRIVATE void SysProcessExit(NX_Error errCode)
{
    NX_ProcessExit(errCode);
    NX_PANIC("process exit syscall failed !");
}

NX_PRIVATE NX_Error SysProcessCreate(char *name, char *path, NX_U32 flags)
{
    return NX_ProcessCreate(name, path, flags);
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

NX_PRIVATE NX_U64 SysVfsRead(int fd, void * buf, NX_U64 len, NX_Error *outErr)
{
    return NX_VfsRead(fd, buf, len, outErr);
}

NX_PRIVATE NX_U64 SysVfsWrite(int fd, void * buf, NX_U64 len, NX_Error *outErr)
{
    return NX_VfsWrite(fd, buf, len, outErr);
}

NX_PRIVATE NX_I64 SysVfsFileSeek(int fd, NX_I64 off, int whence, NX_Error *outErr)
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

/* xbook env syscall table  */
NX_PRIVATE const NX_SyscallHandler NX_SyscallTable[] = 
{
    SysInvalidCall,      /* 0 */
    SysDebugLog,         /* 1 */
    SysProcessExit,
    SysProcessCreate,
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
