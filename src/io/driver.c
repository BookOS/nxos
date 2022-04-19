/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Device
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-3-15      JasonHu           Init
 */

#include <io/driver.h>
#include <xbook/init_call.h>
#include <mm/alloc.h>
#include <utils/string.h>
#include <utils/memory.h>
#include <xbook/debug.h>
#define NX_LOG_NAME "driver"
#include <utils/log.h>

NX_PRIVATE NX_LIST_HEAD(DriverListHead);
NX_PRIVATE NX_SPIN_DEFINE_UNLOCKED(DriverLock);

NX_Driver *NX_DriverCreate(const char *name, NX_DeviceType type, NX_U32 flags, NX_DriverOps *ops)
{
    if (name == NX_NULL || !ops)
    {
        return NX_NULL;
    }

    if (type < NX_DEVICE_TYPE_UNKNOWN || type >= NX_DEVICE_TYPE_NR)
    {
        return NX_NULL;
    }

    NX_Driver *driver = NX_MemAlloc(sizeof(NX_Driver));
    if (driver == NX_NULL)
    {
        return NX_NULL;
    }

    NX_StrCopy(driver->name, name);
    driver->type = type;
    driver->ops = ops;
    driver->flags = flags;
    driver->extension = NX_NULL;

    NX_ListInit(&driver->list);
    NX_ListInit(&driver->deviceListHead);
    NX_SpinInit(&driver->lock);
    return driver;
}

NX_Error NX_DriverDestroy(NX_Driver *driver)
{
    if (!driver)
    {
        return NX_EINVAL;
    }
    NX_MemFree(driver);
    return NX_EOK;
}

NX_Error NX_DriverRegister(NX_Driver *driver)
{
    if (driver == NX_NULL)
    {
        return NX_EINVAL;
    }
    NX_UArch level;
    NX_SpinLockIRQ(&DriverLock, &level);
    
    NX_Driver *tmp;
    NX_ListForEachEntry(tmp, &DriverListHead, list)
    {
        if (!NX_StrCmp(tmp->name, driver->name))
        {
            return NX_EAGAIN; /* meet same driver */
        }
    }
    NX_ListAdd(&driver->list, &DriverListHead);
    NX_SpinUnlockIRQ(&DriverLock, level);
    return NX_EOK;
}

NX_Error NX_DriverUnregister(NX_Driver *driver)
{
    if (driver == NX_NULL)
    {
        return NX_EINVAL;
    }

    NX_UArch level;
    NX_SpinLockIRQ(&DriverLock, &level);
    NX_ListDel(&driver->list);
    NX_SpinUnlockIRQ(&DriverLock, level);
    return NX_EOK;
}

NX_Driver *NX_DriverSearch(const char *name)
{
    if (!name)
    {
        return NX_NULL;
    }
    NX_Driver *tmp;
    NX_UArch level;
    NX_SpinLockIRQ(&DriverLock, &level);
    NX_ListForEachEntry(tmp, &DriverListHead, list)
    {
        if (!NX_StrCmp(tmp->name, name))
        {
            NX_SpinUnlockIRQ(&DriverLock, level);
            return tmp;
        }
    }
    NX_SpinUnlockIRQ(&DriverLock, level);
    return NX_NULL;
}

NX_Error NX_DriverCleanup(const char *name)
{
    if (!name)
    {
        return NX_EINVAL;
    }
    NX_Driver *driver = NX_DriverSearch(name);
    if (driver)
    {
        NX_Device *device, *n;
        NX_ListForEachEntrySafe(device, n, &driver->deviceListHead, list)
        {
            NX_DriverDetachDevice(driver, device->name);
        }
        NX_DriverUnregister(driver);
        NX_DriverDestroy(driver);
        return NX_EOK;
    }
    return NX_EOK;
}

NX_Device *NX_DeviceCreate(const char *name)
{
    if (name == NX_NULL)
    {
        return NX_NULL;
    }

    NX_Device *device = NX_MemAlloc(sizeof(NX_Device));
    if (device == NX_NULL)
    {
        return NX_NULL;
    }

    NX_ListInit(&device->list);
    device->driver = NX_NULL;
    device->extension = NX_NULL;
    NX_StrCopy(device->name, name);
    NX_AtomicSet(&device->reference, 0);
    NX_MutexInit(&device->lock);

    return device;
}

NX_Error NX_DeviceDestroy(NX_Device *device)
{
    if (!device)
    {
        return NX_EINVAL;
    }
    NX_MemFree(device);
    return NX_EOK;
}

NX_PRIVATE NX_Error NX_DriverAttachDeviceObject(NX_Driver *driver, NX_Device *device)
{
    if (!driver || !device)
    {
        return NX_EINVAL;
    }

    NX_UArch level;
    NX_SpinLockIRQ(&driver->lock, &level);

    NX_Device *tmp;
    NX_ListForEachEntry(tmp, &driver->deviceListHead, list)
    {
        if (!NX_StrCmp(tmp->name, device->name))
        {
            return NX_EAGAIN; /* meet same device */
        }
    }

    NX_ListAdd(&device->list, &driver->deviceListHead);
    device->driver = driver;
    NX_SpinUnlockIRQ(&driver->lock, level);
    return NX_EOK;
}

NX_PRIVATE NX_Error NX_DriverDetachDeviceObject(NX_Driver *driver, const char *name, NX_Device **device)
{
    if (!driver || !name || !device)
    {
        return NX_EINVAL;
    }

    NX_UArch level;
    NX_SpinLockIRQ(&driver->lock, &level);
    NX_Device *tmp;
    NX_ListForEachEntry(tmp, &driver->deviceListHead, list)
    {
        if (!NX_StrCmp(tmp->name, name))
        {
            NX_ListDelInit(&tmp->list);
            *device = tmp;
            NX_SpinUnlockIRQ(&driver->lock, level);
            return NX_EOK;
        }
    }
    NX_SpinUnlockIRQ(&driver->lock, level);
    return NX_ENOSRCH;
}

NX_Error NX_DriverAttachDevice(NX_Driver *driver, const char *name, NX_Device **outDevice)
{
    if (!driver || !name)
    {
        return NX_EINVAL;
    }
    NX_Device *device = NX_DeviceCreate(name);
    if (device == NX_NULL)
    {
        return NX_ENOMEM;
    }
    if (outDevice)
    {
        *outDevice = device;
    }
    return NX_DriverAttachDeviceObject(driver, device);
}

NX_Error NX_DriverDetachDevice(NX_Driver *driver, const char *name)
{
    if (!driver || !name)
    {
        return NX_EINVAL;
    }
    
    NX_Device *device = NX_NULL;
    NX_Error err = NX_DriverDetachDeviceObject(driver, name, &device);
    if (err != NX_EOK)
    {
        return err;
    }
    NX_ASSERT(device);
    NX_DeviceDestroy(device);
    return NX_EOK;
}

NX_PRIVATE NX_Device *NX_DeviceSearchLocked(const char *name)
{
    NX_Device *device;
    NX_Driver *driver;
    
    NX_ListForEachEntry(driver, &DriverListHead, list)
    {
        NX_ListForEachEntry(device, &driver->deviceListHead, list)
        {
            if (!NX_StrCmp(device->name, name))
            {
                return device;
            }
        }
    }
    return NX_NULL;
}

NX_Device *NX_DeviceEnum(NX_Offset offset)
{
    NX_Driver *driver;
    NX_Device *device;
    NX_UArch level;
    NX_Offset idx;

    idx = 0;
    NX_SpinLockIRQ(&DriverLock, &level);
    NX_ListForEachEntry(driver, &DriverListHead, list)
    {
        NX_ListForEachEntry(device, &driver->deviceListHead, list)
        {
            if (idx == offset)
            {
                NX_SpinUnlockIRQ(&DriverLock, level);
                return device;
            }
            idx++;
        }
    }
    NX_SpinUnlockIRQ(&DriverLock, level);
    return NX_NULL;
}

NX_Error NX_DeviceOpen(const char *name, NX_U32 flags, NX_Device **outDevice)
{
    if (!name)
    {
        return NX_EINVAL;
    }

    NX_Device *device;
    NX_Driver *driver = NX_NULL;
    
    NX_UArch level;
    NX_SpinLockIRQ(&DriverLock, &level);
    device = NX_DeviceSearchLocked(name);
    if (device)
    {
        NX_AtomicInc(&device->reference);
        NX_SpinUnlockIRQ(&DriverLock, level);
    
        driver = device->driver;

        NX_DriverOps *ops = driver->ops;
        if (ops)
        {
            if (ops->open)
            {
                /* call open if has ope new one device flags  or only one reference */
                if ((driver->flags & NX_DEVICE_OPEN_NEW_ONE) || NX_AtomicGet(&device->reference) == 1)
                {
                    NX_Error err = ops->open(device, flags);
                    if (err != NX_EOK)
                    {
                        return err;
                    }
                }
            }
        }
        if (outDevice)
        {
            *outDevice = device;
        }
        return NX_EOK;
    }
    NX_SpinUnlockIRQ(&DriverLock, level);
    return NX_ENOSRCH;
}

NX_Error NX_DeviceClose(NX_Device *device)
{
    if (!device)
    {
        return NX_EINVAL;
    }

    NX_Driver *driver = device->driver;
    NX_ASSERT(driver);

    if (NX_AtomicGet(&device->reference) <= 0)
    {
        return NX_EPERM;
    }

    NX_AtomicDec(&device->reference);

    NX_MutexLock(&device->lock);

    NX_DriverOps *ops = driver->ops;
    if (ops)
    {
        if (ops->close)
        {
            /* call close if has ope new one device flags or only zero reference */
            if ((driver->flags & NX_DEVICE_OPEN_NEW_ONE) || NX_AtomicGet(&device->reference) == 0)
            {
                NX_Error err = ops->close(device);
                if (err != NX_EOK)
                {
                    NX_MutexUnlock(&device->lock);
                    return err;
                }
            }
        }
    }
    NX_MutexUnlock(&device->lock);
    return NX_EOK;
}

NX_Error NX_DeviceRead(NX_Device *device, void *buf, NX_Size len, NX_Size *outLen)
{
    if (!device || !buf || !len)
    {
        return NX_EINVAL;
    }

    NX_Driver *driver = device->driver;
    NX_ASSERT(driver);

    NX_MutexLock(&device->lock);

    NX_DriverOps *ops = driver->ops;
    NX_Error err = NX_EOK;
    if (ops)
    {
        if (ops->read)
        {
            NX_Size readLen;
            err = ops->read(device, buf, len, &readLen);
            if (err != NX_EOK)
            {
                NX_MutexUnlock(&device->lock);
                return err;
            }
            if (outLen)
            {
                *outLen = readLen;
            }
        }
        else
        {
            err = NX_ENOFUNC;
        }

    }

    NX_MutexUnlock(&device->lock);
    return err;
}

NX_Error NX_DeviceWrite(NX_Device *device, void *buf, NX_Size len, NX_Size *outLen)
{
    if (!device || !buf || !len)
    {
        return NX_EINVAL;
    }

    NX_Driver *driver = device->driver;
    NX_ASSERT(driver);

    NX_MutexLock(&device->lock);

    NX_DriverOps *ops = driver->ops;
    NX_Error err = NX_EOK;
    if (ops)
    {
        if (ops->write)
        {
            NX_Size writeLen;
            err = ops->write(device, buf, len, &writeLen);
            if (err != NX_EOK)
            {            
                NX_MutexUnlock(&device->lock);
                return err;
            }
            if (outLen)
            {
                *outLen = writeLen;
            }
        }
        else
        {
            err = NX_ENOFUNC;
        }
    }
    NX_MutexUnlock(&device->lock);
    return err;
}

NX_Error NX_DeviceReadEx(NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen)
{
    if (!device || !buf || !len)
    {
        return NX_EINVAL;
    }

    NX_Driver *driver = device->driver;
    NX_ASSERT(driver);

    NX_MutexLock(&device->lock);

    NX_DriverOps *ops = driver->ops;
    NX_Error err = NX_EOK;
    if (ops)
    {
        if (ops->readEx)
        {
            NX_Size readLen;
            err = ops->readEx(device, buf, off, len, &readLen);
            if (err != NX_EOK)
            {
                NX_MutexUnlock(&device->lock);
                return err;
            }
            if (outLen)
            {
                *outLen = readLen;
            }
        }
        else
        {
            err = NX_ENOFUNC;
        }

    }

    NX_MutexUnlock(&device->lock);
    return err;
}

NX_Error NX_DeviceWriteEx(NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen)
{
    if (!device || !buf || !len)
    {
        return NX_EINVAL;
    }

    NX_Driver *driver = device->driver;
    NX_ASSERT(driver);

    NX_MutexLock(&device->lock);

    NX_DriverOps *ops = driver->ops;
    NX_Error err = NX_EOK;
    if (ops)
    {
        if (ops->writeEx)
        {
            NX_Size writeLen;
            err = ops->writeEx(device, buf, off, len, &writeLen);
            if (err != NX_EOK)
            {            
                NX_MutexUnlock(&device->lock);
                return err;
            }
            if (outLen)
            {
                *outLen = writeLen;
            }
        }
        else
        {
            err = NX_ENOFUNC;
        }
    }
    NX_MutexUnlock(&device->lock);
    return err;
}

NX_Error NX_DeviceControl(NX_Device *device, NX_U32 cmd, void *arg)
{
    if (!device)
    {
        return NX_EINVAL;
    }

    NX_Driver *driver = device->driver;
    NX_ASSERT(driver);

    NX_MutexLock(&device->lock);

    NX_DriverOps *ops = driver->ops;
    NX_Error err = NX_EOK;
    if (ops)
    {
        if (ops->control)
        {
            err = ops->control(device, cmd, arg);
            if (err != NX_EOK)
            {          
                NX_MutexUnlock(&device->lock);
                return err;
            }
        }
        else
        {
            err = NX_ENOFUNC;
        }
    }
    NX_MutexUnlock(&device->lock);
    return err;
}

NX_Device *NX_DeviceSearch(const char *name)
{
    NX_Device *device;
    NX_UArch level;
    NX_SpinLockIRQ(&DriverLock, &level);
    device = NX_DeviceSearchLocked(name);
    NX_SpinUnlockIRQ(&DriverLock, level);
    return device;
}

void NX_DriverDumpTree(void)
{
    NX_Driver *driver;
    NX_Device *device;

    NX_LOG_I("Driver Tree:");
    NX_ListForEachEntry(driver, &DriverListHead, list)
    {
        NX_LOG_I("    |- driver name: %s, flags: %x", driver->name, driver->flags);
        NX_ListForEachEntry(device, &driver->deviceListHead, list)
        {
            NX_LOG_I("        |- device name: %s, referece: %d", device->name, device->reference);            
        }
    }
}
