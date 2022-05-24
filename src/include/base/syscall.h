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

#ifndef __PROCESS_SYSCALL___
#define __PROCESS_SYSCALL___

#include <nxos.h>

#define NX_SYSCALL_ENV_XBOOK    0
#define NX_SYSCALL_ENV_POSIX    1
#define NX_SYSCALL_ENV_WIN32    2

#define NX_SYSCALL_ENV_SHIFT    24

#define NX_SYSCALL_ENV(api)     ((api) >> NX_SYSCALL_ENV_SHIFT)
#define NX_SYSCALL_NUMBER(api)  ((api) & ((1UL << NX_SYSCALL_ENV_SHIFT) - 1))

typedef void * NX_SyscallHandler;
typedef NX_UArch (*NX_SyscallWithArgHandler) (NX_UArch, NX_UArch, NX_UArch, NX_UArch, NX_UArch, NX_UArch, NX_UArch);

/* 
 * syscall api made with api env and api number.
 * 
 * bit[31~24]:  api env(xbook systemcall, posix subsystem call, win32 subsystem call and so on)
 * bit[23~0]:   api number
 */
typedef NX_U32 NX_SyscallApi;

NX_SyscallHandler NX_SyscallGetHandler(NX_SyscallApi api);

#endif /* __PROCESS_SYSCALL___ */
