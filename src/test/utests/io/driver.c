/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: io drivers test 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-3-16      JasonHu           Init
 */

#include <test/utest.h>
#include <base/driver.h>

#ifdef CONFIG_NX_UTEST_IO_DRIVER

#define DRV_NAME "utest_driver"
#define DRV_NAME2 "utest_driver2"

#define DEV_NAME0 "utest_dev0"
#define DEV_NAME1 "utest_dev1"
#define DEV_NAME2 "utest_dev2"

NX_PRIVATE NX_Error UtestOpen(struct NX_Device *device, NX_U32 flags)
{
    NX_ASSERT_NOT_NULL(device);
    return NX_EOK;
}

NX_PRIVATE NX_Error UtestClose(struct NX_Device *device)
{
    NX_ASSERT_NOT_NULL(device);
    return NX_EOK;
}

NX_PRIVATE NX_Error UtestRead(struct NX_Device *device, void *buf, NX_Size len, NX_Size *outLen)
{
    NX_ASSERT_NOT_NULL(device);
    NX_ASSERT_NOT_NULL(buf);
    NX_ASSERT_GT(len, 0);
    
    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error UtestWrite(struct NX_Device *device, void *buf, NX_Size len, NX_Size *outLen)
{
    NX_ASSERT_NOT_NULL(device);
    NX_ASSERT_NOT_NULL(buf);
    NX_ASSERT_GT(len, 0);
    
    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error UtestControl(struct NX_Device *device, NX_U32 cmd, void *arg)
{
    NX_ASSERT_NOT_NULL(device);

    if (cmd > 5)
    {
        return NX_EINVAL;
    }
    return NX_EOK;
}

NX_PRIVATE NX_DriverOps utestOps = {
    .open = UtestOpen,
    .close = UtestClose,
    .read = UtestRead,
    .write = UtestWrite,
    .control = UtestControl,
};

NX_TEST(NX_DriverCreate)
{
    NX_Driver *driver;
    driver = NX_DriverCreate(NX_NULL, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NULL(driver);
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, NX_NULL);
    NX_ASSERT_NULL(driver);
    driver = NX_DriverCreate(DRV_NAME, -1, 0, &utestOps);
    NX_ASSERT_NULL(driver);
    
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);
    
    NX_ASSERT_EQ(NX_DriverDestroy(driver), NX_EOK);
}

NX_TEST(NX_DriverDestroy)
{
    NX_Driver *driver;
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);

    NX_ASSERT_NE(NX_DriverDestroy(NX_NULL), NX_EOK);
    NX_ASSERT_EQ(NX_DriverDestroy(driver), NX_EOK);
}

NX_TEST(NX_DriverRegister)
{
    NX_Driver *driver;
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);
    
    NX_ASSERT_NE(NX_DriverRegister(NX_NULL), NX_EOK);
    
    NX_ASSERT_EQ(NX_DriverRegister(driver), NX_EOK);
    NX_ASSERT_EQ(NX_DriverUnregister(driver), NX_EOK);

    NX_ASSERT_EQ(NX_DriverRegister(driver), NX_EOK);
    NX_ASSERT_EQ(NX_DriverUnregister(driver), NX_EOK);

    NX_ASSERT_EQ(NX_DriverDestroy(driver), NX_EOK);
}

NX_TEST(NX_DriverUnregister)
{
    NX_Driver *driver;
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);
    
    NX_ASSERT_NE(NX_DriverUnregister(NX_NULL), NX_EOK);
    
    NX_ASSERT_EQ(NX_DriverRegister(driver), NX_EOK);
    NX_ASSERT_EQ(NX_DriverUnregister(driver), NX_EOK);

    NX_ASSERT_EQ(NX_DriverRegister(driver), NX_EOK);
    NX_ASSERT_EQ(NX_DriverUnregister(driver), NX_EOK);
    
    NX_ASSERT_EQ(NX_DriverDestroy(driver), NX_EOK);

    NX_ASSERT_EQ(NX_DriverSearch(DRV_NAME), NX_NULL);
}

NX_TEST(NX_DriverSearch)
{
    NX_Driver *driver;
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);
    
    NX_ASSERT_EQ(NX_DriverRegister(driver), NX_EOK);

    NX_Driver *search = NX_DriverSearch(NX_NULL);
    NX_ASSERT_NULL(search);

    search = NX_DriverSearch(DRV_NAME);
    NX_ASSERT_EQ(search, driver);

    search = NX_DriverSearch(DRV_NAME2);
    NX_ASSERT_EQ(search, NX_NULL);

    NX_ASSERT_EQ(NX_DriverUnregister(driver), NX_EOK);    
    NX_ASSERT_EQ(NX_DriverDestroy(driver), NX_EOK);
}

NX_TEST(NX_DriverCleanup)
{
    NX_Driver *driver;
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);
    
    NX_ASSERT_EQ(NX_DriverRegister(driver), NX_EOK);
    NX_ASSERT_EQ(NX_DriverCleanup(DRV_NAME), NX_EOK);
    
    NX_ASSERT_EQ(NX_DriverSearch(DRV_NAME), NX_NULL);
}

NX_TEST(NX_DriverAttachDevice)
{
    NX_Driver *driver;
    NX_Device *device;
    
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);

    NX_Error err;
    err = NX_DriverAttachDevice(NX_NULL, DEV_NAME0, NX_NULL);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DriverAttachDevice(driver, NX_NULL, NX_NULL);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DriverAttachDevice(NX_NULL, NX_NULL, NX_NULL);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DriverAttachDevice(driver, DEV_NAME0, &device);
    NX_ASSERT_EQ(err, NX_EOK);
    NX_ASSERT_NOT_NULL(device);
    
    err = NX_DriverAttachDevice(driver, DEV_NAME1, &device);
    NX_ASSERT_EQ(err, NX_EOK);
    NX_ASSERT_NOT_NULL(device);

    err = NX_DriverDetachDevice(driver, DEV_NAME0);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DriverDetachDevice(driver, DEV_NAME1);
    NX_ASSERT_EQ(err, NX_EOK);

    NX_ASSERT_EQ(NX_DriverDestroy(driver), NX_EOK);
}

NX_TEST(NX_DriverDetachDevice)
{
    NX_Driver *driver;
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);

    NX_Error err;
    err = NX_DriverDetachDevice(NX_NULL, DEV_NAME0);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DriverDetachDevice(driver, NX_NULL);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DriverDetachDevice(NX_NULL, NX_NULL);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DriverAttachDevice(driver, DEV_NAME0, NX_NULL);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DriverAttachDevice(driver, DEV_NAME1, NX_NULL);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DriverDetachDevice(driver, DEV_NAME0);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DriverDetachDevice(driver, DEV_NAME1);
    NX_ASSERT_EQ(err, NX_EOK);

    NX_ASSERT_EQ(NX_DriverDestroy(driver), NX_EOK);
}

NX_TEST(NX_DeviceSearch)
{
    NX_Driver *driver;
    NX_Device *device;
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);

    NX_Error err;
    err = NX_DriverAttachDevice(driver, DEV_NAME0, NX_NULL);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DriverAttachDevice(driver, DEV_NAME1, NX_NULL);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DriverRegister(driver);
    NX_ASSERT_EQ(err, NX_EOK);

    device = NX_DeviceSearch(DEV_NAME0);
    NX_ASSERT_NOT_NULL(device);

    device = NX_DeviceSearch(DEV_NAME1);
    NX_ASSERT_NOT_NULL(device);

    device = NX_DeviceSearch(DEV_NAME2);
    NX_ASSERT_NULL(device);

    NX_ASSERT_EQ(NX_DriverCleanup(DRV_NAME), NX_EOK);

    device = NX_DeviceSearch(DEV_NAME0);
    NX_ASSERT_NULL(device);

    device = NX_DeviceSearch(DEV_NAME1);
    NX_ASSERT_NULL(device);
}

NX_TEST(NX_DeviceOpen)
{
    NX_Driver *driver;
    NX_Device *device0, *device1, *device2;
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);

    NX_Error err;
    err = NX_DriverAttachDevice(driver, DEV_NAME0, NX_NULL);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DriverAttachDevice(driver, DEV_NAME1, NX_NULL);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DriverRegister(driver);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DeviceOpen(NX_NULL, 0, NX_NULL);
    NX_ASSERT_NE(err, NX_EOK);
    
    device0 = NX_NULL;
    err = NX_DeviceOpen(DEV_NAME0, 0, &device0);
    NX_ASSERT_EQ(err, NX_EOK);
    NX_ASSERT_NOT_NULL(device0);

    device1 = NX_NULL;
    err = NX_DeviceOpen(DEV_NAME1, 0, &device1);
    NX_ASSERT_EQ(err, NX_EOK);
    NX_ASSERT_NOT_NULL(device1);
    
    device2 = NX_NULL;
    err = NX_DeviceOpen(DEV_NAME2, 0, &device2);
    NX_ASSERT_NE(err, NX_EOK);
    NX_ASSERT_NULL(device2);

    err = NX_DeviceClose(device0);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DeviceClose(device1);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DeviceClose(device2);
    NX_ASSERT_NE(err, NX_EOK);

    NX_ASSERT_EQ(NX_DriverCleanup(DRV_NAME), NX_EOK);
}

NX_TEST(NX_DeviceClose)
{
    NX_Driver *driver;
    NX_Device *device;
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);

    NX_Error err;
    err = NX_DriverAttachDevice(driver, DEV_NAME0, NX_NULL);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DriverRegister(driver);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DeviceOpen(DEV_NAME0, 0, &device);
    NX_ASSERT_EQ(err, NX_EOK);
    NX_ASSERT_NOT_NULL(device);

    err = NX_DeviceClose(device);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DeviceClose(NX_NULL);
    NX_ASSERT_NE(err, NX_EOK);

    NX_ASSERT_EQ(NX_DriverCleanup(DRV_NAME), NX_EOK);
}

NX_TEST(NX_DeviceRead)
{
    NX_Driver *driver;
    NX_Device *device;
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);

    NX_Error err;
    err = NX_DriverAttachDevice(driver, DEV_NAME0, NX_NULL);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DriverRegister(driver);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DeviceOpen(DEV_NAME0, 0, &device);
    NX_ASSERT_EQ(err, NX_EOK);
    NX_ASSERT_NOT_NULL(device);

    NX_Size len;
    char buf[32] = {0};

    err = NX_DeviceRead(NX_NULL, buf, 1, &len);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DeviceRead(device, NX_NULL, 1, &len);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DeviceRead(device, NX_NULL, 0, &len);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DeviceRead(device, buf, 0, &len);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DeviceRead(device, buf, 1, &len);
    NX_ASSERT_EQ(err, NX_EOK);
    NX_ASSERT_EQ(len, 1);

    err = NX_DeviceRead(device, buf, 100, &len);
    NX_ASSERT_EQ(err, NX_EOK);
    NX_ASSERT_EQ(len, 100);

    err = NX_DeviceRead(device, buf, 1, NX_NULL);
    NX_ASSERT_EQ(err, NX_EOK);
    
    err = NX_DeviceClose(device);
    NX_ASSERT_EQ(err, NX_EOK);

    NX_ASSERT_EQ(NX_DriverCleanup(DRV_NAME), NX_EOK);
}

NX_TEST(NX_DeviceWrite)
{
    NX_Driver *driver;
    NX_Device *device;
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);

    NX_Error err;
    err = NX_DriverAttachDevice(driver, DEV_NAME0, NX_NULL);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DriverRegister(driver);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DeviceOpen(DEV_NAME0, 0, &device);
    NX_ASSERT_EQ(err, NX_EOK);
    NX_ASSERT_NOT_NULL(device);

    NX_Size len;
    char buf[32] = {0};
    
    err = NX_DeviceWrite(NX_NULL, buf, 1, &len);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DeviceWrite(device, NX_NULL, 1, &len);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DeviceWrite(device, NX_NULL, 0, &len);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DeviceWrite(device, buf, 0, &len);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DeviceWrite(device, buf, 1, &len);
    NX_ASSERT_EQ(err, NX_EOK);
    NX_ASSERT_EQ(len, 1);

    err = NX_DeviceWrite(device, buf, 100, &len);
    NX_ASSERT_EQ(err, NX_EOK);
    NX_ASSERT_EQ(len, 100);

    err = NX_DeviceWrite(device, buf, 1, NX_NULL);
    NX_ASSERT_EQ(err, NX_EOK);
    
    err = NX_DeviceClose(device);
    NX_ASSERT_EQ(err, NX_EOK);

    NX_ASSERT_EQ(NX_DriverCleanup(DRV_NAME), NX_EOK);
}

NX_TEST(NX_DeviceControl)
{
    NX_Driver *driver;
    NX_Device *device;
    driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_UNKNOWN, 0, &utestOps);
    NX_ASSERT_NOT_NULL(driver);

    NX_Error err;
    err = NX_DriverAttachDevice(driver, DEV_NAME0, NX_NULL);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DriverRegister(driver);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DeviceOpen(DEV_NAME0, 0, &device);
    NX_ASSERT_EQ(err, NX_EOK);
    NX_ASSERT_NOT_NULL(device);

    char buf[32] = {0};
    
    err = NX_DeviceControl(NX_NULL, 0, NX_NULL);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DeviceControl(device, 0, NX_NULL);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DeviceControl(device, 1, buf);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DeviceControl(device, 4, buf);
    NX_ASSERT_EQ(err, NX_EOK);

    err = NX_DeviceControl(device, 6, buf);
    NX_ASSERT_NE(err, NX_EOK);

    err = NX_DeviceClose(device);
    NX_ASSERT_EQ(err, NX_EOK);

    NX_ASSERT_EQ(NX_DriverCleanup(DRV_NAME), NX_EOK);
}

NX_TEST_TABLE(NX_Driver)
{
    NX_TEST_UNIT(NX_DriverCreate),
    NX_TEST_UNIT(NX_DriverDestroy),
    NX_TEST_UNIT(NX_DriverRegister),
    NX_TEST_UNIT(NX_DriverUnregister),
    NX_TEST_UNIT(NX_DriverSearch),
    NX_TEST_UNIT(NX_DriverCleanup),
    NX_TEST_UNIT(NX_DriverAttachDevice),
    NX_TEST_UNIT(NX_DriverDetachDevice),
    NX_TEST_UNIT(NX_DeviceSearch),
    NX_TEST_UNIT(NX_DeviceOpen),
    NX_TEST_UNIT(NX_DeviceClose),
    NX_TEST_UNIT(NX_DeviceRead),
    NX_TEST_UNIT(NX_DeviceWrite),
    NX_TEST_UNIT(NX_DeviceControl),
};

NX_TEST_CASE_SETUP(NX_Driver)
{
    /* check driver */
    NX_ASSERT_NULL(NX_DriverSearch(DRV_NAME));
    NX_ASSERT_NULL(NX_DeviceSearch(DEV_NAME0));
    NX_ASSERT_NULL(NX_DeviceSearch(DEV_NAME0));    
    return NX_EOK;
}

NX_TEST_CASE_CLEAN(NX_Driver)
{
    /* check driver */
    NX_ASSERT_NULL(NX_DriverSearch(DRV_NAME));
    NX_ASSERT_NULL(NX_DeviceSearch(DEV_NAME0));
    NX_ASSERT_NULL(NX_DeviceSearch(DEV_NAME0));
    return NX_EOK;
}

NX_TEST_CASE(NX_Driver);

#endif
