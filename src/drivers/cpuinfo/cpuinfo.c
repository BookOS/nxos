/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: cpu info driver
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-05-12     JasonHu           Init
 */

#include <base/driver.h>

#ifdef CONFIG_NX_DRIVER_CPUINFO

#define NX_LOG_NAME "cpu info driver"
#include <base/log.h>
#include <base/memory.h>
#include <base/smp.h>
#include <base/uaccess.h>

#define DRV_NAME "cpu info device"
#define DEV_NAME "cpuinfo"

#define NX_CPUINFO_GET_CORES 1

typedef struct NX_CpuInfo
{
    NX_U32 usage[NX_MULTI_CORES_NR];
} NX_CpuInfo;

NX_PRIVATE NX_Error CpuInfoRead(struct NX_Device *device, void *buf, NX_Size len, NX_Size *outLen)
{
    NX_CpuInfo cpuinfo;
    int i;

    if (len != sizeof(NX_CpuInfo))
    {
        return NX_EINVAL;
    }

    for (i = 0; i < NX_MULTI_CORES_NR; i++)
    {
        cpuinfo.usage[i] = NX_SMP_GetUsage(i);
    }

    NX_CopyToUser(buf, (char *)&cpuinfo, len);

    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error CpuInfoControl(struct NX_Device *device, NX_U32 cmd, void *arg)
{
    if (cmd != NX_CPUINFO_GET_CORES)
    {
        return NX_EINVAL;
    }
    NX_U32 cores = NX_MULTI_CORES_NR;

    NX_CopyToUser(arg, (char *)&cores, sizeof(cores));

    return NX_EOK;
}

NX_PRIVATE NX_DriverOps CpuInfoDriverOps = {
    .read = CpuInfoRead,
    .control = CpuInfoControl,
};

NX_PRIVATE void CpuInfoDriverInit(void)
{
    NX_Device *device;
    NX_Driver *driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_VIRT, 0, &CpuInfoDriverOps);
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

NX_PRIVATE void CpuInfoDriverExit(void)
{
    NX_DriverCleanup(DRV_NAME);
}

NX_DRV_INIT(CpuInfoDriverInit);
NX_DRV_EXIT(CpuInfoDriverExit);

#endif
