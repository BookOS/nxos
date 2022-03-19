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

NX_PRIVATE int NX_SysInvalidCall(void)
{
    NX_Thread *cur = NX_ThreadSelf();
    NX_LOG_E("thread %s/%d call invalid syscall!", cur->name, cur->tid);    
    return 0;
}

NX_PRIVATE int NX_SysDebugLog(const char *buf, int size)
{
    /* FIXME: check buf -> buf + size accessable */
    NX_Printf(buf);
    return 0;
}

NX_PRIVATE void NX_SysProcessExit(NX_Error errCode)
{
    NX_ProcessExit(errCode);
    NX_PANIC("process exit syscall failed !");
}

NX_PRIVATE NX_Error NX_SysProcessCreate(char *name, char *path, NX_U32 flags)
{
    return NX_ProcessCreate(name, path, flags);
}

/* xbook env syscall table  */
NX_PRIVATE const NX_SyscallHandler NX_SyscallTable[] = 
{
    NX_SysInvalidCall,    /* 0 */
    NX_SysDebugLog,       /* 1 */
    NX_SysProcessExit,
    NX_SysProcessCreate,
};

/* posix env syscall table */
NX_PRIVATE const NX_SyscallHandler NX_SyscallTablePosix[] = 
{
    NX_SysInvalidCall,    /* 0 */
};

NX_PRIVATE const NX_SyscallHandler NX_SyscallTableWin32[] = 
{
    NX_SysInvalidCall,    /* 0 */
};

NX_SyscallHandler NX_SyscallGetHandler(NX_SyscallApi api)
{
    NX_SyscallHandler handler = NX_SysInvalidCall;

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
