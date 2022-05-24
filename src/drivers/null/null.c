/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Null driver
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-3-16      JasonHu           Init
 */

#include <base/driver.h>

#ifdef CONFIG_NX_DRIVER_NULL

#define NX_LOG_NAME "null driver"
#include <base/log.h>

#define DRV_NAME "null device"
#define DEV_NAME "null"

NX_PRIVATE NX_Error NullWrite(struct NX_Device *device, void *buf, NX_Size len, NX_Size *outLen)
{
    /* do nothing for write */

    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_DriverOps NullDriverOps = {
    .write = NullWrite,
};

NX_PRIVATE void NullDriverInit(void)
{
    NX_Device *device;
    NX_Driver *driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_VIRT, 0, &NullDriverOps);
    if (driver == NX_NULL)
    {
        NX_LOG_E("create driver failed!");
        return;
    }

    if (NX_DriverAttachDevice(driver, DEV_NAME, &device) != NX_EOK)
    {
        NX_LOG_E("attach device %s failed!", DEV_NAME);
        NX_DriverDestroy(driver);
        return;
    }

    if (NX_DriverRegister(driver) != NX_EOK)
    {
        NX_LOG_E("register driver %s failed!", DRV_NAME);
        NX_DriverDetachDevice(driver, DEV_NAME);
        NX_DriverDestroy(driver);
        return;
    }
    
    NX_LOG_I("init %s driver success!", DRV_NAME);
}

NX_PRIVATE void NullDriverExit(void)
{
    NX_DriverCleanup(DRV_NAME);
}

NX_DRV_INIT(NullDriverInit);
NX_DRV_EXIT(NullDriverExit);

#endif
