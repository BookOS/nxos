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

#include <xbook/exobj.h>
#include <mm/alloc.h>
#include <xbook/debug.h>

NX_Error NX_ExposedObjectTableInit(NX_ExposedObjectTable * table, NX_Size count)
{
    int i;

    if (!table || !count || count > NX_EXOBJ_MAX_NR)
    {
        return NX_EINVAL;
    }
    table->objectCount = count;
    table->objects = NX_MemAlloc(sizeof(NX_ExposedObject) * count);
    if (table->objects == NX_NULL)
    {
        return NX_ENOMEM;
    }
    NX_SpinInit(&table->lock);
    for (i = 0; i < count; i++)
    {
        table->objects[i].object = NX_NULL;
        table->objects[i].type = NX_EXOBJ_NONE;
        table->objects[i].close = NX_NULL;
    }
    return NX_EOK;
}

/**
 * exit object table no need free table, because just exit table, not destroy table
 */
NX_Error NX_ExposedObjectTableExit(NX_ExposedObjectTable * table)
{
    NX_Solt solt;

    if (!table)
    {
        return NX_EINVAL;
    }

    /* uninstall all objects */
    for (solt = 0; solt < table->objectCount; solt++)
    {
        NX_ExposedObjectUninstalll(table, solt);
    }

    NX_MemFree(table->objects);
    return NX_EOK;
}

NX_Solt NX_ExposedObjectAlloc(NX_ExposedObjectTable * table)
{
    NX_ASSERT(table);
    
    NX_Solt i;
    NX_ExposedObject * objects = table->objects;

    for (i = 0; i < table->objectCount; i++)
    {
        if (objects[i].object == NX_NULL)
        {
            return i;
        }
    }
    return -1;
}

NX_ExposedObject * NX_ExposedObjectGet(NX_ExposedObjectTable * table, NX_Solt solt)
{
    if (!table)
    {
        return NX_NULL;
    }

    NX_ExposedObject * objects = table->objects;

    if (solt < table->objectCount)
    {
        return &objects[solt];
    }

    return NX_NULL;
}

NX_Solt NX_ExposedObjectLocate(NX_ExposedObjectTable * table, void * object, NX_ExposedObjectType type)
{
    NX_Solt solt;
    NX_UArch level;

    if (!table || !object)
    {
        return NX_SOLT_INVALID_VALUE;
    }

    NX_ExposedObject * objects = table->objects;

    NX_SpinLockIRQ(&table->lock, &level);
    for (solt = 0; solt < table->objectCount; solt++)
    {
        if (objects[solt].object == object && objects[solt].type == type)
        {
            NX_SpinUnlockIRQ(&table->lock, level);
            return solt;
        }
    }
    NX_SpinUnlockIRQ(&table->lock, level);
    return NX_SOLT_INVALID_VALUE;
}

NX_Error NX_ExposedObjectInstall(NX_ExposedObjectTable * table, void * object, NX_ExposedObjectType type, NX_SoltCloseHandler handler, NX_Solt * outSolt)
{
    NX_Solt solt;
    NX_ExposedObject * objects = NX_NULL;
    NX_UArch level;

    if (!table)
    {
        return NX_EINVAL;
    }
    
    objects = table->objects;
    if (!objects)
    {
        return NX_EFAULT;
    }

    NX_SpinLockIRQ(&table->lock, &level);
    solt = NX_ExposedObjectAlloc(table);
    if (solt < 0)
    {
        NX_SpinUnlockIRQ(&table->lock, level);
        return NX_ENORES;
    }

    objects[solt].object = object;
    objects[solt].type = type;
    objects[solt].close = handler;
    
    if (outSolt)
    {
        *outSolt = solt;
    }

    NX_SpinUnlockIRQ(&table->lock, level);
    return NX_EOK;
}

NX_Error NX_ExposedObjectUninstalll(NX_ExposedObjectTable * table, NX_Solt solt)
{
    NX_ExposedObject * objects = NX_NULL;
    NX_UArch level;
    NX_Error err;

    if (!table)
    {
        return NX_EINVAL;
    }
    
    objects = table->objects;
    if (!objects)
    {
        return NX_EFAULT;
    }

    if (solt >= table->objectCount)
    {
        return NX_EINVAL;
    }

    if (objects->object == NX_NULL)
    {
        return NX_EFAULT;
    }

    if (objects->close)
    {
        err = objects->close(objects->object, objects->type);
        if (err != NX_EOK)
        {
            return err;
        }
    }

    NX_SpinLockIRQ(&table->lock, &level);

    objects[solt].object = NX_NULL;
    objects[solt].type = NX_EXOBJ_NONE;
    objects[solt].close = NX_NULL;

    NX_SpinUnlockIRQ(&table->lock, level);
    return NX_EOK;    
}

NX_Error NX_ExposedObjectCopy(NX_ExposedObjectTable * dst, NX_ExposedObjectTable * src, NX_Solt solt, NX_Solt * outSolt)
{
    NX_ExposedObject * exobj;

    if (!dst || !src)
    {
        return NX_EINVAL;
    }

    exobj = NX_ExposedObjectGet(src, solt);
    if (!exobj)
    {
        return NX_EFAULT;
    }

    return NX_ExposedObjectInstall(dst, exobj->object, exobj->type, exobj->close, outSolt);
}
