/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: physical mem driver
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-06-19     JasonHu           Init
 */

#include <base/driver.h>

#ifdef CONFIG_NX_DRIVER_PMEM

#include <base/page.h>
#include <base/uaccess.h>
#include <drvfw/pmem.h>

#define NX_LOG_NAME "pmem driver"
#include <base/log.h>

#define DRV_NAME "phy mem"
#define DEV_NAME "pmem"

typedef struct DeviceExtension
{
    NX_Addr phyAddr;
} DeviceExtension;

NX_PRIVATE DeviceExtension phyMemDevice;

NX_PRIVATE NX_Error PmemRead(struct NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen)
{
    NX_Addr paddr;
    char * vaddr;

    paddr = (NX_Addr)off;

    vaddr = (char *)NX_Phy2Virt(paddr);
    if (!vaddr)
    {
        return NX_EINVAL;
    }
    NX_CopyToUser(buf, vaddr, len);

    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error PmemWrite(struct NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen)
{
    NX_Addr paddr;
    char * vaddr;

    paddr = (NX_Addr)off;

    vaddr = (char *)NX_Phy2Virt(paddr);
    if (!vaddr)
    {
        return NX_EINVAL;
    }
    
    NX_CopyFromUser(vaddr, buf, len);

    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error PmemControl(struct NX_Device *device, NX_U32 cmd, void *arg)
{
    DeviceExtension * devext;

    devext = (DeviceExtension *)device->extension;

    switch (cmd)
    {
    case NX_PMEM_CMD_SETADDR:
        {
            if (!arg)
            {
                return NX_EINVAL;
            }
            NX_CopyFromUser((char *)&devext->phyAddr, (char *)arg, sizeof(NX_Addr));
        }
        break;
    
    case NX_PMEM_CMD_GETADDR:
        {
            if (!arg)
            {
                return NX_EINVAL;
            }
            NX_CopyToUser((char *)arg, (char *)&devext->phyAddr, sizeof(NX_Addr));
        }
        break;
    
    default:
        return NX_EINVAL;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error PmemMappable(struct NX_Device *device, NX_Size length, NX_U32 prot, NX_Addr * outPhyAddr)
{
    DeviceExtension * devext;

    devext = (DeviceExtension *)device->extension;

    *outPhyAddr = devext->phyAddr;

    return NX_EOK;
}

NX_PRIVATE NX_DriverOps pmemDriverOps = {
    .read = PmemRead,
    .write = PmemWrite,
    .control = PmemControl,
    .mappable = PmemMappable,
};

NX_PRIVATE void PmemDriverInit(void)
{
    NX_Device *device;
    NX_Driver *driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_VIRT, 0, &pmemDriverOps);
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
    
    device->extension = &phyMemDevice;

    phyMemDevice.phyAddr = 0;

    if (NX_DriverRegister(driver) != NX_EOK)
    {
        NX_LOG_E("register driver %s failed!", DRV_NAME);
        NX_DriverDetachDevice(driver, DEV_NAME);
        NX_DriverDestroy(driver);
        return;
    }
}

NX_PRIVATE void PmemDriverExit(void)
{
    NX_DriverCleanup(DRV_NAME);
}

NX_DRV_INIT(PmemDriverInit);
NX_DRV_EXIT(PmemDriverExit);

#endif
