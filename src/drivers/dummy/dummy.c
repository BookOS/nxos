/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: dummy driver
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-3-15      JasonHu           Init
 */

#include <base/driver.h>

#ifdef CONFIG_NX_DRIVER_DUMMY

#define NX_LOG_NAME "dummy driver"
#include <base/log.h>

#define DRV_NAME "dummy device"
#define DEV0_NAME "dummy0"
#define DEV1_NAME "dummy1"

NX_PRIVATE NX_Error DummyOpen(struct NX_Device *device, NX_U32 flags)
{
    return NX_EOK;
}

NX_PRIVATE NX_Error DummyClose(struct NX_Device *device)
{
    return NX_EOK;
}

NX_PRIVATE NX_Error DummyRead(struct NX_Device *device, void *buf, NX_Size len, NX_Size *outLen)
{
    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error DummyWrite(struct NX_Device *device, void *buf, NX_Size len, NX_Size *outLen)
{
    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error DummyControl(struct NX_Device *device, NX_U32 cmd, void *arg)
{
    return NX_EOK;
}

NX_PRIVATE NX_DriverOps DummyDriverOps = {
    .open = DummyOpen,
    .close = DummyClose,
    .read = DummyRead,
    .write = DummyWrite,
    .control = DummyControl,
};

NX_PRIVATE void DummyDriverInit(void)
{
    NX_Device *device;
    NX_Driver *driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_VIRT, 0, &DummyDriverOps);
    if (driver == NX_NULL)
    {
        NX_LOG_E("create driver failed!");
        return;
    }

    if (NX_DriverAttachDevice(driver, DEV0_NAME, &device) != NX_EOK)
    {
        NX_LOG_E("attach device %s failed!", DEV0_NAME);
        NX_DriverDestroy(driver);
        return;
    }

    if (NX_DriverAttachDevice(driver, DEV1_NAME, &device) != NX_EOK)
    {
        NX_LOG_E("attach device %s failed!", DEV1_NAME);
        NX_DriverDetachDevice(driver, DEV0_NAME);
        NX_DriverDestroy(driver);
        return;
    }

    if (NX_DriverRegister(driver) != NX_EOK)
    {
        NX_LOG_E("register driver %s failed!", DRV_NAME);
        NX_DriverDetachDevice(driver, DEV0_NAME);
        NX_DriverDetachDevice(driver, DEV1_NAME);
        NX_DriverDestroy(driver);
        return;
    }
    
    NX_LOG_I("init %s driver success!", DRV_NAME);
}

NX_PRIVATE void DummyDriverExit(void)
{
    NX_DriverCleanup(DRV_NAME);
}

NX_DRV_INIT(DummyDriverInit);
NX_DRV_EXIT(DummyDriverExit);

#endif
