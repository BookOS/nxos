/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: kernel virtual mem driver
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-06-20      JasonHu           Init
 */

#include <base/driver.h>

#ifdef CONFIG_NX_DRIVER_VMEM

#include <base/page.h>
#include <base/uaccess.h>
#include <arch/process.h>
#include <drvfw/pmem.h>

#define NX_LOG_NAME "vmem driver"
#include <base/log.h>

#define DRV_NAME "kernel virtual mem"
#define DEV_NAME "vmem"

NX_PRIVATE NX_Error VmemRead(struct NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen)
{
    NX_Addr vaddr;

    vaddr = (NX_Addr)off;

    /* check kernel addr */
    if (((vaddr >= NX_USER_SPACE_VADDR) && (vaddr < NX_USER_SPACE_VADDR + NX_USER_SPACE_SIZE)))
    {
        NX_LOG_E("vmem write addr %p not in kernel!", vaddr);
        return NX_EINVAL;
    }

    NX_CopyToUser((char *)buf, (char *)vaddr, len);

    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error VmemWrite(struct NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen)
{
    NX_Addr vaddr;

    vaddr = (NX_Addr)off;

    /* check kernel addr */
    if (((vaddr >= NX_USER_SPACE_VADDR) && (vaddr < NX_USER_SPACE_VADDR + NX_USER_SPACE_SIZE)))
    {
        NX_LOG_E("vmem write addr %p not in kernel!", vaddr);
        return NX_EINVAL;
    }

    NX_CopyFromUser((char *)vaddr, (char *)buf, len);

    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_DriverOps vmemDriverOps = {
    .read = VmemRead,
    .write = VmemWrite,
};

NX_PRIVATE void VmemDriverInit(void)
{
    NX_Device *device;
    NX_Driver *driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_VIRT, 0, &vmemDriverOps);
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
}

NX_PRIVATE void VmemDriverExit(void)
{
    NX_DriverCleanup(DRV_NAME);
}

NX_DRV_INIT(VmemDriverInit);
NX_DRV_EXIT(VmemDriverExit);

#endif
