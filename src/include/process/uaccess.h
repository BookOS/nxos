/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Process user acess
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-7       JasonHu           Init
 */

#ifndef __PROCESS_UACCESS_H___
#define __PROCESS_UACCESS_H___

#include <utils/memory.h>

NX_INLINE NX_Error NX_CopyFromUser(char *kernelBuf, char *userBuf, NX_Size size)
{
	/* validate userBuf */
	NX_MemCopy(kernelBuf, userBuf, size);
    return NX_EOK;
}

NX_INLINE NX_Error NX_CopyToUser(char *userBuf, char *kernelBuf, NX_Size size)
{
	/* validate userBuf */
	NX_MemCopy(userBuf, kernelBuf, size);
    return NX_EOK;
}

#define NX_CopyFromUserEx(kernel, user) NX_CopyFromUser((char *)(kernel), (char *)(user), sizeof(*(kernel)))
#define NX_CopyToUserEx(user, kernel) NX_CopyToUser((char *)(user), (char *)(kernel), sizeof(*(kernel)))

#endif /* __PROCESS_UACCESS_H___ */
