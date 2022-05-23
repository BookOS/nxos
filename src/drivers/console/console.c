/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Console driver
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-20      JasonHu           Init
 */

#include <base/driver.h>

#define NX_LOG_NAME "console driver"
#include <base/log.h>
#include <base/memory.h>
#include <base/string.h>
#include <base/ioqueue.h>

/* Conosle weak function */
NX_INTERFACE NX_WEAK_SYM void NX_ConsoleSendData(char ch) {}

#ifdef CONFIG_NX_DRIVER_CONSOLE
NX_PRIVATE NX_IoQueue * consoleDataQueue = NX_NULL;
#endif

void NX_ConsoleReceveData(char ch)
{
#ifdef CONFIG_NX_DRIVER_CONSOLE
    NX_IoQueueTryPut(consoleDataQueue, ch);
#endif
}

#ifdef CONFIG_NX_DRIVER_CONSOLE

#define DRV_NAME "console driver"
#define DEV_NAME "console"

#define CONS_IN_BUF_LEN 8

NX_PRIVATE NX_Error ConsoleOpen(struct NX_Device *device, NX_U32 flags)
{
    return NX_EOK;
}

NX_PRIVATE NX_Error ConsoleClose(struct NX_Device *device)
{
    return NX_EOK;
}

NX_PRIVATE NX_Error ConsoleRead(struct NX_Device *device, void *buf, NX_Size len, NX_Size *outLen)
{
    char *p;
    int i;
    NX_Error err = NX_EOK;

    p = buf;
    for (i = 0; i < len; i++)
    {
        *p = NX_IoQueueGet(consoleDataQueue, &err);
        if (err != NX_EOK)
        {
            if (outLen)
            {
                *outLen = i;
            }
            return err;
        }
        p++;
    }
    
    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error ConsoleWrite(struct NX_Device *device, void *buf, NX_Size len, NX_Size *outLen)
{
    char *p;
    int i;

    p = buf;
    for (i = 0; i < len; i++)
    {    
        NX_ConsoleSendData(*p++);
    }
    
    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error ConsoleControl(struct NX_Device *device, NX_U32 cmd, void *arg)
{
    return NX_EOK;
}

NX_PRIVATE NX_DriverOps consoleDriverOps = {
    .open = ConsoleOpen,
    .close = ConsoleClose,
    .read = ConsoleRead,
    .write = ConsoleWrite,
    .control = ConsoleControl,
};

NX_PRIVATE void ConsoleDriverInit(void)
{
    NX_Error err = NX_EOK;
    NX_Device *device = NX_NULL;
    NX_Driver *driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_CHAR, 0, &consoleDriverOps);
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
    
    consoleDataQueue = NX_IoQueueCreate(CONS_IN_BUF_LEN, &err);
    if (consoleDataQueue == NX_NULL)
    {
        NX_LOG_E("driver %s create io queue failed!", DRV_NAME);
        NX_DriverCleanup(DRV_NAME);
        return;
    }

    NX_LOG_I("init %s driver success!", DRV_NAME);
}

NX_PRIVATE void ConsoleDriverExit(void)
{
    NX_DriverCleanup(DRV_NAME);
}

NX_DRV_INIT(ConsoleDriverInit);
NX_DRV_EXIT(ConsoleDriverExit);

#endif
