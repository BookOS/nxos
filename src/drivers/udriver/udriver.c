/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: udriver driver
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-3-15      JasonHu           Init
 */

#include <base/driver.h>

#define NX_LOG_NAME "udriver"
#include <base/log.h>
#include <base/hub.h>
#include <base/string.h>
#include <base/malloc.h>
#include <base/debug.h>
#include <base/uaccess.h>

#define UDRIVER_HUB_PREFIX "udrv_"

typedef struct DriverExtension
{
    NX_UserDriver * udrv; /* user driver */
    char hubName[NX_HUB_NAME_LEN];   /* udriver hub name */
} DriverExtension;

enum UdriverOpsAPI
{
    UDRIVER_OPS_OPEN = 0,
    UDRIVER_OPS_CLOSE,
    UDRIVER_OPS_READ,
    UDRIVER_OPS_WRITE,
    UDRIVER_OPS_CONTROL,
    UDRIVER_OPS_MAPPABLE,
    UDRIVER_OPS_POLL,
};

NX_PRIVATE NX_Error UdriverOpen(struct NX_Device *device, NX_U32 flags)
{
    NX_Error err;
    NX_Driver * kdrv;
    NX_UserDevice * udev;
    NX_Size retVal = 0;
    NX_HubParam param;
    DriverExtension * drvext;

    kdrv = device->driver;
    udev = (NX_UserDevice * )device->extension;
    drvext = (DriverExtension * )kdrv->extension;

    param.api = UDRIVER_OPS_OPEN;
    param.args[0] = (NX_Size)drvext->udrv;
    param.args[1] = (NX_Size)udev;
    param.args[2] = (NX_Size)flags;

    if ((err = NX_HubCallParamName(drvext->hubName, &param, &retVal)) != NX_EOK)
    {
        return err;
    }

    return retVal;
}

NX_PRIVATE NX_Error UdriverClose(struct NX_Device *device)
{
    NX_Error err;
    NX_Driver * kdrv;
    NX_UserDevice * udev;
    NX_Size retVal = 0;
    NX_HubParam param;
    DriverExtension * drvext;

    kdrv = device->driver;
    udev = (NX_UserDevice * )device->extension;
    drvext = (DriverExtension * )kdrv->extension;

    param.api = UDRIVER_OPS_CLOSE;
    param.args[0] = (NX_Size)drvext->udrv;
    param.args[1] = (NX_Size)udev;

    if ((err = NX_HubCallParamName(drvext->hubName, &param, &retVal)) != NX_EOK)
    {
        return err;
    }

    return retVal;
}

NX_PRIVATE NX_Error UdriverRead(struct NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen)
{
    NX_Error err;
    NX_Driver * kdrv;
    NX_UserDevice * udev;
    NX_Size retVal = 0;
    NX_HubParam param;
    DriverExtension * drvext;

    kdrv = device->driver;
    udev = (NX_UserDevice * )device->extension;
    drvext = (DriverExtension * )kdrv->extension;

    param.api = UDRIVER_OPS_READ;
    param.args[0] = (NX_Size)drvext->udrv;
    param.args[1] = (NX_Size)udev;
    param.args[2] = (NX_Size)buf;
    param.args[3] = (NX_Size)off;
    param.args[4] = (NX_Size)len;
    param.args[5] = (NX_Size)outLen;

    if ((err = NX_HubCallParamName(drvext->hubName, &param, &retVal)) != NX_EOK)
    {
        return err;
    }
    return retVal;
}

NX_PRIVATE NX_Error UdriverWrite(struct NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen)
{
    NX_Error err;
    NX_Driver * kdrv;
    NX_UserDevice * udev;
    NX_Size retVal = 0;
    NX_HubParam param;
    DriverExtension * drvext;

    kdrv = device->driver;
    udev = (NX_UserDevice * )device->extension;
    drvext = (DriverExtension * )kdrv->extension;

    param.api = UDRIVER_OPS_WRITE;
    param.args[0] = (NX_Size)drvext->udrv;
    param.args[1] = (NX_Size)udev;
    param.args[2] = (NX_Size)buf;
    param.args[3] = (NX_Size)off;
    param.args[4] = (NX_Size)len;
    param.args[5] = (NX_Size)outLen;

    if ((err = NX_HubCallParamName(drvext->hubName, &param, &retVal)) != NX_EOK)
    {
        return err;
    }
    return retVal;
}

NX_PRIVATE NX_Error UdriverControl(struct NX_Device *device, NX_U32 cmd, void *arg)
{
    NX_Error err;
    NX_Driver * kdrv;
    NX_UserDevice * udev;
    NX_Size retVal = 0;
    NX_HubParam param;
    DriverExtension * drvext;

    kdrv = device->driver;
    udev = (NX_UserDevice * )device->extension;
    drvext = (DriverExtension * )kdrv->extension;

    param.api = UDRIVER_OPS_CONTROL;
    param.args[0] = (NX_Size)drvext->udrv;
    param.args[1] = (NX_Size)udev;
    param.args[2] = (NX_Size)cmd;
    param.args[3] = (NX_Size)arg;

    if ((err = NX_HubCallParamName(drvext->hubName, &param, &retVal)) != NX_EOK)
    {
        return err;
    }

    return retVal;
}

NX_PRIVATE NX_Error UdriverMappable(struct NX_Device *device, NX_Size length, NX_U32 prot, NX_Addr * outPhyAddr)
{
    NX_Error err;
    NX_Driver * kdrv;
    NX_UserDevice * udev;
    NX_Size retVal = 0;
    NX_HubParam param;
    DriverExtension * drvext;

    kdrv = device->driver;
    udev = (NX_UserDevice * )device->extension;
    drvext = (DriverExtension * )kdrv->extension;

    param.api = UDRIVER_OPS_MAPPABLE;
    param.args[0] = (NX_Size)drvext->udrv;
    param.args[1] = (NX_Size)udev;
    param.args[2] = (NX_Size)length;
    param.args[3] = (NX_Size)prot;
    param.args[4] = (NX_Size)outPhyAddr;

    if ((err = NX_HubCallParamName(drvext->hubName, &param, &retVal)) != NX_EOK)
    {
        return err;
    }

    return retVal;
}

NX_PRIVATE NX_Error UdriverPoll(struct NX_Device *device, NX_PollState * pState)
{
    NX_Error err;
    NX_Driver * kdrv;
    NX_UserDevice * udev;
    NX_Size retVal = 0;
    NX_HubParam param;
    DriverExtension * drvext;

    kdrv = device->driver;
    udev = (NX_UserDevice * )device->extension;
    drvext = (DriverExtension * )kdrv->extension;

    param.api = UDRIVER_OPS_POLL;
    param.args[0] = (NX_Size)drvext->udrv;
    param.args[1] = (NX_Size)udev;
    param.args[2] = (NX_Size)pState;

    if ((err = NX_HubCallParamName(drvext->hubName, &param, &retVal)) != NX_EOK)
    {
        return err;
    }

    return retVal;
}

NX_PRIVATE NX_DriverOps UdriverDriverOps = {
    .open = UdriverOpen,
    .close = UdriverClose,
    .read = UdriverRead,
    .write = UdriverWrite,
    .control = UdriverControl,
    .mappable = UdriverMappable,
    .poll = UdriverPoll,
};

NX_PRIVATE NX_Error UdriverUnregister(NX_Driver * drv)
{
    DriverExtension * drvext;
    NX_Error err;

    if (!drv)
    {
        return NX_EINVAL;
    }

    drvext = (DriverExtension * )drv->extension;

    if ((err = NX_HubUnregister(drvext->hubName)) != NX_EOK)
    {
        NX_LOG_W("udriver %s unregister hub %s failed with error %s!", drv->name, drvext->hubName, NX_ErrorToString(err));
        return err;
    }
    
    NX_Device *device, *n;
    NX_ListForEachEntrySafe(device, n, &drv->deviceListHead, list)
    {
        NX_DriverDetachDevice(drv, device->name);
    }
    if (drv->extension)
    {
        NX_MemFree(drv->extension);
    }
    NX_DriverUnregister(drv);
    NX_DriverDestroy(drv);
    return NX_EOK;
}

NX_PRIVATE NX_Error UdriverCloseSolt(void * object, NX_ExposedObjectType type)
{
    NX_Driver * drv;

    if (type != NX_EXOBJ_UDRIVER)
    {
        return NX_ENORES;
    }

    drv = (NX_Driver *) object;
    NX_ASSERT(drv);

    return UdriverUnregister(drv);
}

NX_Error NX_UserDriverRegister(NX_UserDriver * drv, NX_Solt * outSolt)
{
    NX_Solt solt = NX_SOLT_INVALID_VALUE;
    NX_Error err;
    DriverExtension * drvext;
    NX_Device *device;
    NX_Size deviceCount;
    char hubName[NX_HUB_NAME_LEN + 1] = {0};

    if (!drv)
    {
        return NX_EINVAL;
    }

    /* check driver registered */
    if (NX_DriverSearch(drv->name) != NX_NULL)
    {
        NX_LOG_E("driver %s has already registered", drv->name);
        return NX_EPERM;
    }

    NX_Driver *driver = NX_DriverCreate(drv->name, drv->type, drv->flags, &UdriverDriverOps);
    if (driver == NX_NULL)
    {
        NX_LOG_E("create udriver %s failed!", drv->name);
        return NX_ENOMEM;
    }

    drvext = NX_MemAlloc(sizeof(DriverExtension));
    if (drvext == NX_NULL)
    {
        NX_LOG_E("alloc udriver %s extension failed!", drv->name);
        NX_DriverDestroy(driver);
        return NX_ENOMEM;
    }
    
    drvext->udrv = drv;
    NX_StrCopy(hubName, UDRIVER_HUB_PREFIX);
    NX_StrCat(hubName, drv->name);
    NX_StrCopy(drvext->hubName, hubName);

    driver->extension = drvext;

    NX_UserDevice * userDev;

    deviceCount = 0;
    NX_ListForEachEntry (userDev, &drv->deviceListHead, list)
    {
        if ((err = NX_DriverAttachDevice(driver, userDev->name, &device)) != NX_EOK)
        {
            NX_LOG_E("udriver %s attach device %s failed with err %s!", drv->name, userDev->name, NX_ErrorToString(err));
            NX_MemFree(drvext);
            NX_DriverCleanup(drv->name);
            return NX_ENOMEM;
        }
        deviceCount++;
        device->extension = userDev;
    }

    if ((err = NX_DriverRegister(driver)) != NX_EOK)
    {
        NX_LOG_E("udriver %s register failed with err %s!", drv->name, NX_ErrorToString(err));
        NX_MemFree(drvext);
        NX_DriverCleanup(drv->name);
        return err;
    }
    
    /* register hub server */
    if ((err = NX_HubRegister(hubName, deviceCount, NX_NULL)) != NX_EOK)
    {
        NX_LOG_E("udriver %s register hub %s failed with err %s!", drv->name, hubName, NX_ErrorToString(err));
        NX_MemFree(drvext);
        NX_DriverCleanup(drv->name);
        return err;
    }

    /* install udriver exobj */
    if (NX_ProcessInstallSolt(NX_ProcessCurrent(), driver, NX_EXOBJ_UDRIVER, UdriverCloseSolt, &solt) != NX_EOK)
    {
        NX_LOG_E("udriver %s install failed with err %s!", drv->name, NX_ErrorToString(err));
        NX_HubUnregister(hubName);
        NX_MemFree(drvext);
        NX_DriverCleanup(drv->name);
        return err;
    }

    if (outSolt)
    {
        NX_CopyToUser((char *)outSolt, (char *)&solt, sizeof(solt));
    }

    return err;
}
