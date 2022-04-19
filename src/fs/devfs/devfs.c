/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: device vfs interface.
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-19      JasonHu           Init
 */

#include <xbook.h>

#include <fs/vfs.h>
#include <utils/string.h>
#include <utils/memory.h>
#include <utils/log.h>
#include <io/block.h>

NX_PRIVATE NX_Error DevMount(NX_VfsMount * m, const char * dev)
{
	m->root->data = (void *)NX_NULL;
	m->data = NX_NULL;
	return NX_EOK;
}

NX_PRIVATE NX_Error DevUnmount(NX_VfsMount * m)
{
	m->data = NX_NULL;
	return NX_EOK;
}

NX_PRIVATE NX_Error DevMountSync(NX_VfsMount * m)
{
	return NX_EOK;
}

NX_PRIVATE NX_Error DevVget(NX_VfsMount * m, NX_VfsNode * n)
{
	return NX_EOK;
}

NX_PRIVATE NX_Error DevVput(NX_VfsMount * m, NX_VfsNode * n)
{
	return NX_EOK;
}

NX_PRIVATE NX_U64 DevRead(NX_VfsNode * n, NX_I64 off, void * buf, NX_U64 len, NX_Error *outErr)
{
	NX_Device *dev;
	NX_Size outLen;
	NX_Error err;

	dev = n->data;

	outLen = 0;
	if (dev->driver->ops->readEx)
	{
		err = NX_DeviceReadEx(dev, buf, off, len, &outLen);
		if (err != NX_EOK)
		{
			NX_ErrorSet(outErr, err);
			return 0;
		}
	}
	else if (dev->driver->ops->read)
	{
		err = NX_DeviceRead(dev, buf, len, &outLen);
		if (err != NX_EOK)
		{
			NX_ErrorSet(outErr, err);
			return 0;
		}
	}
	else
	{
		NX_ErrorSet(outErr, NX_ENOFUNC);
		return 0;
	}
	NX_ErrorSet(outErr, NX_EOK);
	return outLen;
}

NX_PRIVATE NX_U64 DevWrite(NX_VfsNode * n, NX_I64 off, void * buf, NX_U64 len, NX_Error *outErr)
{
    NX_Device *dev;
	NX_Size outLen;
	NX_Error err;

	dev = n->data;

	outLen = 0;
	if (dev->driver->ops->writeEx)
	{
		err = NX_DeviceWriteEx(dev, buf, off, len, &outLen);
		if (err != NX_EOK)
		{
			NX_ErrorSet(outErr, err);
			return 0;
		}
	}
	else if (dev->driver->ops->write)
	{
		err = NX_DeviceWrite(dev, buf, len, &outLen);
		if (err != NX_EOK)
		{
			NX_ErrorSet(outErr, err);
			return 0;
		}
	}
	else
	{
		NX_ErrorSet(outErr, NX_ENOFUNC);
		return 0;
	}
	NX_ErrorSet(outErr, NX_EOK);
	return outLen;
}

NX_PRIVATE NX_Error DevIoctl(NX_VfsNode * n, NX_U32 cmd, void *arg)
{
    NX_Device *dev;

    dev = n->data;

    return NX_DeviceControl(dev, cmd, arg);
}

NX_PRIVATE NX_Error DevTruncate(NX_VfsNode * n, NX_I64 off)
{
	return NX_EPERM; /* can't truncate */
}

NX_PRIVATE NX_Error DevSync(NX_VfsNode * n)
{
	return NX_EOK;
}

NX_PRIVATE NX_Error DevReaddir(NX_VfsNode * dn, NX_I64 off, NX_VfsDirent * d)
{
    NX_Device *device;

	device = NX_DeviceEnum(off);
	if (device == NX_NULL)
    {
        return NX_ENORES;
    }

    if (device->driver->type == NX_DEVICE_TYPE_BLOCK)
    {
        d->type = NX_VFS_DIR_TYPE_BLK;
    }
    else
    {
        d->type = NX_VFS_DIR_TYPE_CHR;
    }
    
	NX_StrCopyN(d->name, device->name, sizeof(d->name));
    d->name[sizeof(device->name) - 1] = '\0';

	d->off = off;
	d->reclen = 1;

    return NX_EOK;
}

NX_PRIVATE NX_Error DevLookup(NX_VfsNode * dn, const char * name, NX_VfsNode * n)
{
	NX_Device *dev;

	dev = NX_DeviceSearch(name);
	if (dev == NX_NULL)
	{
		return NX_ENOSRCH;
	}

	n->atime = 0;
	n->mtime = 0;
	n->ctime = 0;
	n->mode = 0;
	n->size = 0;
	n->data = (void *)dev;

	n->type = NX_VFS_NODE_TYPE_REG;
	n->mode |= NX_VFS_S_IFREG;
	if(dev->driver->ops->read || dev->driver->ops->readEx)
	{
		n->mode |= (NX_VFS_S_IRUSR | NX_VFS_S_IRGRP | NX_VFS_S_IROTH);
	}
	if(dev->driver->ops->write || dev->driver->ops->writeEx)
	{
		n->mode |= (NX_VFS_S_IWUSR | NX_VFS_S_IWGRP | NX_VFS_S_IWOTH);
	}

	return NX_EOK;
}

NX_PRIVATE NX_Error DevCreate(NX_VfsNode * dn, const char * filename, NX_U32 mode)
{
	return NX_EPERM;
}

NX_PRIVATE NX_Error DevRemove(NX_VfsNode * dn, NX_VfsNode * n, const char *name)
{
	return NX_EPERM;
}

NX_PRIVATE NX_Error DevRename(NX_VfsNode * sn, const char * sname, NX_VfsNode * n, NX_VfsNode * dn, const char * dname)
{
	return NX_EPERM;
}

NX_PRIVATE NX_Error DevMkdir(NX_VfsNode * dn, const char * name, NX_U32 mode)
{
	return NX_EPERM;
}

NX_PRIVATE NX_Error DevRmdir(NX_VfsNode * dn, NX_VfsNode * n, const char *name)
{
	return NX_EPERM;
}

NX_PRIVATE NX_Error DevChmod(NX_VfsNode * n, NX_U32 mode)
{
	return NX_EPERM;
}

NX_PRIVATE NX_VfsFileSystem devfs = {
	.name		= "devfs",

	.mount		= DevMount,
	.unmount	= DevUnmount,
	.msync		= DevMountSync,
	.vget		= DevVget,
	.vput		= DevVput,

	.read		= DevRead,
	.write		= DevWrite,
    .ioctl      = DevIoctl,
	.truncate	= DevTruncate,
	.sync		= DevSync,
	.readdir	= DevReaddir,
	.lookup		= DevLookup,
	.create		= DevCreate,
	.remove		= DevRemove,
	.rename		= DevRename,
	.mkdir		= DevMkdir,
	.rmdir		= DevRmdir,
	.chmod		= DevChmod,
};

NX_PRIVATE void NX_FileSystemDevInit(void)
{
	NX_VfsRegisterFileSystem(&devfs);
}

NX_PRIVATE void NX_FileSystemDevExit(void)
{
	NX_VfsUnregisterFileSystem(&devfs);
}

NX_FS_INIT(NX_FileSystemDevInit);
NX_FS_EXIT(NX_FileSystemDevExit);
