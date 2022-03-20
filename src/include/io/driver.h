/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: device header
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-3-15      JasonHu           Init
 */

#ifndef __XBOOK_IO_DRIVER__
#define __XBOOK_IO_DRIVER__

#include <xbook.h>
#include <utils/list.h>
#include <sched/spin.h>
#include <sched/mutex.h>
#include <xbook/atomic.h>
#include <xbook/init_call.h>

#define NX_DEVICE_NAME_LEN 32

/* device driver flags */
#define NX_DEVICE_OPEN_NEW_ONE  0x01 /* Every time NX_DeviceOpen is called, the open operation of ops will be called */

enum NX_DeviceType
{
    NX_DEVICE_TYPE_UNKNOWN = 0,
    NX_DEVICE_TYPE_BLOCK,
    NX_DEVICE_TYPE_CHAR,
    NX_DEVICE_TYPE_NET,
    NX_DEVICE_TYPE_UART,
    NX_DEVICE_TYPE_VIRT,
    NX_DEVICE_TYPE_SOUND,
    NX_DEVICE_TYPE_SCREEN,
    NX_DEVICE_TYPE_INPUT,
    NX_DEVICE_TYPE_NR, /* device type number */
};
typedef enum NX_DeviceType NX_DeviceType;

struct NX_Device;

/* device operation set */
struct NX_DriverOps
{
    NX_Error (*open)(struct NX_Device *device, NX_U32 flags);
    NX_Error (*close)(struct NX_Device *device);
    NX_Error (*read)(struct NX_Device *device, void *buf, NX_Size len, NX_Size *outLen);
    NX_Error (*write)(struct NX_Device *device, void *buf, NX_Size len, NX_Size *outLen);
    NX_Error (*readEx)(struct NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen);
    NX_Error (*writeEx)(struct NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen);
    NX_Error (*control)(struct NX_Device *device, NX_U32 cmd, void *arg);
};
typedef struct NX_DriverOps NX_DriverOps;

struct NX_Driver
{
    NX_List list;
    NX_List deviceListHead;
    NX_DeviceType type;
    char name[NX_DEVICE_NAME_LEN];
    NX_DriverOps *ops;
    NX_Spin lock;
    NX_U32 flags;
    void *extension;
};
typedef struct NX_Driver NX_Driver;

struct NX_Device
{
    NX_List list; /* device list */    
    NX_Driver *driver;
    NX_Atomic reference; /* reference counter */
    NX_Mutex lock;
    char name[NX_DEVICE_NAME_LEN];
    void *extension;
};
typedef struct NX_Device NX_Device;

void NX_DriverDumpTree(void);

NX_Driver *NX_DriverCreate(const char *name, NX_DeviceType type, NX_U32 flags, NX_DriverOps *ops);
NX_Error NX_DriverDestroy(NX_Driver *driver);
NX_Error NX_DriverRegister(NX_Driver *driver);
NX_Error NX_DriverUnregister(NX_Driver *driver);
NX_Driver *NX_DriverSearch(const char *name);
NX_Error NX_DriverCleanup(const char *name);

NX_Error NX_DriverAttachDevice(NX_Driver *driver, const char *name, NX_Device **outDevice);
NX_Error NX_DriverDetachDevice(NX_Driver *driver, const char *name);

NX_Error NX_DeviceOpen(const char *name, NX_U32 flags, NX_Device **outDevice);
NX_Error NX_DeviceClose(NX_Device *device);
NX_Error NX_DeviceRead(NX_Device *device, void *buf, NX_Size len, NX_Size *outLen);
NX_Error NX_DeviceWrite(NX_Device *device, void *buf, NX_Size len, NX_Size *outLen);
NX_Error NX_DeviceReadEx(NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen);
NX_Error NX_DeviceWriteEx(NX_Device *device, void *buf, NX_Offset off, NX_Size len, NX_Size *outLen);
NX_Error NX_DeviceControl(NX_Device *device, NX_U32 cmd, void *arg);
NX_Device *NX_DeviceSearch(const char *name);

#endif  /* __XBOOK_IO_DRIVER__ */
