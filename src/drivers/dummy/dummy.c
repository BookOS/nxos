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

#include <io/driver.h>
#define NX_LOG_NAME "dummy driver"
#include <utils/log.h>

#define DRV_NAME "dummy"
#define DEV0_NAME "dummy0"
#define DEV1_NAME "dummy1"

NX_PRIVATE NX_Error DummyOpen(struct NX_Device *device, NX_U32 flags)
{
    NX_LOG_D("Dummy open");
    return NX_EOK;
}

NX_PRIVATE NX_Error DummyClose(struct NX_Device *device)
{
    NX_LOG_D("Dummy close");
    return NX_EOK;
}

NX_PRIVATE NX_DriverOps DummyDriverOps = {
    .open = DummyOpen,
    .close = DummyClose,
};

NX_PRIVATE void DummyDriverInit(void)
{
    NX_Driver *driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_VIRT, 0, &DummyDriverOps);
    if (driver == NX_NULL)
    {
        NX_LOG_E("create driver failed!");
        return;
    }

    if (NX_DriverAttachDevice(driver, DEV0_NAME) != NX_EOK)
    {
        NX_LOG_E("attach device %s failed!", DEV0_NAME);
        NX_DriverDestroy(driver);
        return;
    }

    if (NX_DriverAttachDevice(driver, DEV1_NAME) != NX_EOK)
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
    NX_Driver *driver = NX_DriverSearch(DRV_NAME);
    if (driver)
    {
        NX_Device *device, *n;
        NX_ListForEachEntrySafe(device, n, &driver->deviceListHead, list)
        {
            NX_DriverDetachDevice(driver, device->name);
        }
    }
}

NX_DRV_INIT(DummyDriverInit);
NX_DRV_EXIT(DummyDriverExit);
