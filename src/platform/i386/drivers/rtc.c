/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: rtc driver
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-15      JasonHu           Init
 */

#include <io/driver.h>

#define NX_LOG_NAME "rtc driver"
#include <utils/log.h>
#include <utils/memory.h>
#include <time/time.h>

#include <io.h>

#define DRV_NAME "rtc device"
#define DEV_NAME "rtc"

/* CMOS port */
#define CMOS_INDEX      0x70
#define CMOS_DATA       0x71

/* CMOS offset */
#define CMOS_CUR_SEC	0x0
#define CMOS_ALA_SEC	0x1
#define CMOS_CUR_MIN	0x2
#define CMOS_ALA_MIN	0x3
#define CMOS_CUR_HOUR	0x4
#define CMOS_ALA_HOUR	0x5
#define CMOS_WEEK_DAY	0x6
#define CMOS_MON_DAY	0x7
#define CMOS_CUR_MON	0x8
#define CMOS_CUR_YEAR	0x9
#define CMOS_DEV_TYPE	0x12
#define CMOS_CUR_CEN	0x32

#define BCD_HEX(n)	((n >> 4) * 10) + (n & 0xf)

#define BCD_ASCII_FIRST(n)	(((n << 4) >> 4) + 0x30)
#define BCD_ASCII_S(n)	((n << 4) + 0x30)

NX_PRIVATE NX_U8 ReadCMOS(NX_U8 p)
{
	NX_U8 data;
	IO_Out8(CMOS_INDEX, p);
	data = IO_In8(CMOS_DATA);
	IO_Out8(CMOS_INDEX, 0x80);
	return data;
}

NX_PRIVATE NX_U32 CMOS_GetHourHex(void)
{
	return BCD_HEX(ReadCMOS(CMOS_CUR_HOUR));
}

NX_PRIVATE NX_U32 CMOS_GetMinHex(void)
{
	return BCD_HEX(ReadCMOS(CMOS_CUR_MIN));
}

NX_PRIVATE NX_USED NX_U8 CMOS_GetMinHex8(void)
{
	return BCD_HEX(ReadCMOS(CMOS_CUR_MIN));
}

NX_PRIVATE NX_U32 CMOS_GetSecHex(void)
{
	return BCD_HEX(ReadCMOS(CMOS_CUR_SEC));
}

NX_PRIVATE NX_U32 CMOS_GetDayOfMonth(void)
{
	return BCD_HEX(ReadCMOS(CMOS_MON_DAY));
}

NX_PRIVATE NX_USED NX_U32 CMOS_GetDayOfWeek(void)
{
	return BCD_HEX(ReadCMOS(CMOS_WEEK_DAY));
}

NX_PRIVATE NX_U32 CMOS_GetMonHex(void)
{
	return BCD_HEX(ReadCMOS(CMOS_CUR_MON));
}

NX_PRIVATE NX_U32 CMOS_GetYear(void)
{
	return (BCD_HEX(ReadCMOS(CMOS_CUR_CEN))*100) + \
		BCD_HEX(ReadCMOS(CMOS_CUR_YEAR))+1980;
}

NX_PRIVATE NX_Error RTC_Read(struct NX_Device *device, void *buf, NX_Size len, NX_Size *outLen)
{
    NX_Time time;

    if (len != sizeof(time))
    {
        return NX_EINVAL;
    }

    time.year   = CMOS_GetYear();
    time.month  = CMOS_GetMonHex();
    time.day    = CMOS_GetDayOfMonth();
    time.hour   = CMOS_GetHourHex();
    time.minute = CMOS_GetMinHex();
    time.second = CMOS_GetSecHex();

    NX_MemCopy(buf, &time, sizeof(time));
    
    if (outLen)
    {
        *outLen = len;
    }
    return NX_EOK;
}

NX_PRIVATE NX_DriverOps RTC_DriverOps = {
    .read = RTC_Read,
};

NX_PRIVATE void RTC_DriverInit(void)
{
    NX_Device *device;
    NX_Driver *driver = NX_DriverCreate(DRV_NAME, NX_DEVICE_TYPE_VIRT, 0, &RTC_DriverOps);
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

NX_PRIVATE void RTC_DriverExit(void)
{
    NX_DriverCleanup(DRV_NAME);
}

NX_DRV_INIT(RTC_DriverInit);
NX_DRV_EXIT(RTC_DriverExit);
