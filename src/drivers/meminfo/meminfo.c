/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Memory info driver
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-05-12     JasonHu           Init
 */

#include <base/driver.h>

#ifdef CONFIG_NX_DRIVER_MEMINFO

#define NX_LOG_NAME "mem info driver"
#include <base/log.h>
#include <base/memory.h>
#include <base/page.h>
#include <base/uaccess.h>

#define DRV_NAME "mem info device"
#define DEV_NAME "meminfo"

typedef struct NX_MemInfo
{
    NX_Size pageSize;
    NX_Size totalPage;
    NX_Size usedPage;
} NX_MemInfo;

NX_PRIVATE NX_Error MemInfoRead(struct NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen)
{
    NX_MemInfo meminfo;
    
    if (len != sizeof(NX_MemInfo))
    {
        return NX_EINVAL;
    }

    meminfo.pageSize = NX_PAGE_SIZE;
    meminfo.totalPage = NX_PageGetTotal();
    meminfo.usedPage = NX_PageGetUsed();

    NX_CopyToUser(buf, (char *)&meminfo, len);

    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_DriverOps MemInfoDriverOps = {
    .read = MemInfoRead,
};

NX_PRIVATE void MemInfoDriverInit(void)
{
    NX_Device *device;
    NX_Driver *driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_VIRT, 0, &MemInfoDriverOps);
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

NX_PRIVATE void MemInfoDriverExit(void)
{
    NX_DriverCleanup(DRV_NAME);
}

NX_DRV_INIT(MemInfoDriverInit);
NX_DRV_EXIT(MemInfoDriverExit);

#endif
