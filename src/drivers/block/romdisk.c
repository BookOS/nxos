/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: romdisk from cpio binary
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-3-21      JasonHu           Init
 */

#include <io/driver.h>
#include <io/block.h>
#include <utils/memory.h>

#define NX_LOG_NAME "romdisk driver"
#include <utils/log.h>

#define DRV_NAME "romdisk device"
#define DEV0_NAME "romdisk0"

NX_IMPORT NX_U8 __NX_RomdiskStart[];
NX_IMPORT NX_U8 __NX_RomdiskEnd[];

NX_PRIVATE NX_Size RomdiskSize = 0;

NX_PRIVATE NX_Error RomdiskReadEx(struct NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen)
{
    NX_MemCopy((void *)buf, (const void *)(__NX_RomdiskStart + off), len);
    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error RomdiskControl(struct NX_Device *device, NX_U32 cmd, void *arg)
{
    NX_IoBlockInfo *info = NX_NULL;
    switch (cmd)
    {
    case NX_IO_BLOCK_INFO:
        info = (NX_IoBlockInfo *)arg;
        info->capacity = RomdiskSize;
        info->blockSize = 512;
        info->blockCount = info->capacity / info->blockSize + 1;
        break;
    case NX_IO_BLOCK_SYNC:
        /* no sync action */
        break;
    default:
        break;
    }
    return NX_EOK;
}

NX_PRIVATE NX_DriverOps RomdiskDriverOps = {
    .readEx = RomdiskReadEx,
    .control = RomdiskControl,
};

NX_PRIVATE void RomdiskDriverInit(void)
{
    NX_Device *device;
    NX_Driver *driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_BLOCK, 0, &RomdiskDriverOps);
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

    if (NX_DriverRegister(driver) != NX_EOK)
    {
        NX_LOG_E("register driver %s failed!", DRV_NAME);
        NX_DriverDetachDevice(driver, DEV0_NAME);
        NX_DriverDestroy(driver);
        return;
    }
    
    RomdiskSize = (NX_Size)(__NX_RomdiskEnd - __NX_RomdiskStart);
    NX_LOG_I("romdisk size %x", RomdiskSize);

    NX_LOG_I("init %s driver success!", DRV_NAME);
}

NX_PRIVATE void RomdiskDriverExit(void)
{
    NX_DriverCleanup(DRV_NAME);
}

NX_DRV_INIT(RomdiskDriverInit);
NX_DRV_EXIT(RomdiskDriverExit);
