/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: share memory
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-07-09     JasonHu           Init
 */

#ifndef __MM_SHAREMEM_H__
#define __MM_SHAREMEM_H__

#include <nxos.h>
#include <base/list.h>
#include <base/atomic.h>

#define NX_SHAREMEM_MAX_SIZE (16 * NX_MB)
#define NX_SHAREMEM_NAME_LEN 32

#define NX_SHAREMEM_CREATE_NEW  0x01 /* create share memory new */

typedef struct NX_ShareMem
{
    NX_List list;
    NX_Addr pageAddr;
    NX_Size size;           /* share memory size */
    NX_Atomic reference;    /* share memory map referents */
    char name[NX_SHAREMEM_NAME_LEN];
} NX_ShareMem;

NX_ShareMem * NX_ShareMemOpen(const char * name, NX_Size size, NX_U32 flags);
NX_Error NX_ShareMemClose(NX_ShareMem * shm);

NX_Error NX_ShareMemMap(NX_ShareMem * shm, void ** outMapAddr);
NX_Error NX_ShareMemUnmap(void * addr);

#endif /* __MM_SHAREMEM_H__ */
