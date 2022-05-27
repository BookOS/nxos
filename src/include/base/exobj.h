/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Kernel exposed object to user
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-30      JasonHu           Init
 */

#ifndef __EXPOSED_OBJECT_H__
#define __EXPOSED_OBJECT_H__

#include <nxos.h>
#include <base/spin.h>

#define NX_EXOBJ_MAX_NR 128

#define NX_EXOBJ_DEFAULT_NR 32

#define NX_SOLT_INVALID_VALUE -1

typedef int NX_Solt;

typedef enum NX_ExposedObjectType
{
    NX_EXOBJ_NONE = 0,
    NX_EXOBJ_PORCESS,
    NX_EXOBJ_THREAD,
    NX_EXOBJ_SNAPSHOT,
    NX_EXOBJ_MUTEX,
    NX_EXOBJ_SEMAPHORE,
    NX_EXOBJ_DEVICE,
    NX_EXOBJ_TYPE_NR,
} NX_ExposedObjectType;

typedef NX_Error (*NX_SoltCloseHandler)(void * object, NX_ExposedObjectType type);

typedef struct NX_ExposedObject
{
    void * object;
    NX_ExposedObjectType type;
    NX_SoltCloseHandler close;
} NX_ExposedObject;

typedef struct NX_ExposedObjectTable
{
    NX_Size objectCount;
    NX_ExposedObject * objects;
    NX_Spin lock;
} NX_ExposedObjectTable;

NX_Error NX_ExposedObjectTableInit(NX_ExposedObjectTable * table, NX_Size count);
NX_Error NX_ExposedObjectTableExit(NX_ExposedObjectTable * table);

NX_ExposedObject * NX_ExposedObjectGet(NX_ExposedObjectTable * table, NX_Solt solt);
NX_Solt NX_ExposedObjectLocate(NX_ExposedObjectTable * table, void * object, NX_ExposedObjectType type);

NX_Error NX_ExposedObjectInstall(NX_ExposedObjectTable * table, void * object, NX_ExposedObjectType type, NX_SoltCloseHandler handler, NX_Solt * outSolt);
NX_Error NX_ExposedObjectUninstalll(NX_ExposedObjectTable * table, NX_Solt solt);

NX_Error NX_ExposedObjectCopy(NX_ExposedObjectTable * dst, NX_ExposedObjectTable * src, NX_Solt solt, NX_Solt * outSolt);

NX_Error NX_ExposedObjectClose(NX_ExposedObjectTable * table, NX_Solt solt);

#endif  /* __EXPOSED_OBJECT_H__ */