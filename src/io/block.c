/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Block device
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-3-20      JasonHu           Init
 */

#include <base/block.h>
#include <base/log.h>

NX_Error NX_IoBlockSearch(const char *name)
{
    if (!name)
    {
        return NX_EINVAL;
    }
    NX_Device *dev = NX_DeviceSearch(name);
    if (!dev)
    {
        return NX_ENOSRCH;
    }
    if (dev->driver->type != NX_DEVICE_TYPE_BLOCK)
    {
        return NX_ENOSRCH;
    }
    return NX_EOK;
}

NX_Device *NX_IoBlockOpen(const char *name)
{
    if (!name)
    {
        return NX_NULL;
    }
    NX_Device *dev = NX_NULL;
    if (NX_DeviceOpen(name, 0, &dev) != NX_EOK)
    {
        return NX_NULL;
    }
    return dev;
}

NX_Error NX_IoBlockClose(NX_Device *blk)
{
    if (!blk)
    {
        return NX_EINVAL;
    }
    return NX_DeviceClose(blk);
}

NX_Size NX_IoBlockCapacity(NX_Device *blk)
{
    if (!blk)
    {
        return 0;
    }
    NX_IoBlockInfo info;
    NX_Error err = NX_DeviceControl(blk, NX_IO_BLOCK_INFO, &info);
    if (err != NX_EOK)
    {
        NX_LOG_E("io block capacity failed! %s", NX_ErrorToString(err));
        return 0;
    }

    return info.capacity;
}

NX_Size NX_IoBlockRead(NX_Device *blk, NX_U8 * buf, NX_Offset offset, NX_Size count)
{
    if (!blk || !buf || !count)
    {
        return 0;
    }
    NX_Size ret = 0;
    NX_Error err = NX_DeviceRead(blk, buf, offset, count, &ret);
    if (err != NX_EOK)
    {
        NX_LOG_E("io block read failed! %s", NX_ErrorToString(err));
        ret = 0;
    }
    return ret;
}

NX_Size NX_IoBlockWrite(NX_Device *blk, NX_U8 * buf, NX_Offset offset, NX_Size count)
{
    if (!blk || !buf || !count)
    {
        return 0;
    }
    NX_Size ret = 0;
    NX_Error err = NX_DeviceWrite(blk, buf, offset, count, &ret);
    if (err != NX_EOK)
    {
        NX_LOG_E("io block write failed! %s", NX_ErrorToString(err));
        ret = 0;
    }
    return ret;    
}

void NX_IoBlockSync(NX_Device *blk)
{
    NX_DeviceControl(blk, NX_IO_BLOCK_SYNC, NX_NULL);
}
