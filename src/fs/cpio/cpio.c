/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Copyright(c) 2007-2022 Jianjun Jiang <8192542@qq.com>
 *          Official site: http://xboot.org
 *          Mobile phone: +86-18665388956
 *          QQ: 8192542
 * 
 * Contains: virtual file system form xboot.
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-3-26      JasonHu           Port from xboot
 */

#include <xbook.h>
#include <fs/vfs.h>
#include <utils/string.h>
#include <utils/memory.h>
#include <utils/log.h>
#include <io/block.h>

struct CpioNewcHeader
{
	NX_U8 c_magic[6];
	NX_U8 c_ino[8];
	NX_U8 c_mode[8];
	NX_U8 c_uid[8];
	NX_U8 c_gid[8];
	NX_U8 c_nlink[8];
	NX_U8 c_mtime[8];
	NX_U8 c_filesize[8];
	NX_U8 c_devmajor[8];
	NX_U8 c_devminor[8];
	NX_U8 c_rdevmajor[8];
	NX_U8 c_rdevminor[8];
	NX_U8 c_namesize[8];
	NX_U8 c_check[8];
} NX_PACKED;

NX_PRIVATE NX_Bool GetNextToken(const char * path, const char * prefix, char * result)
{
	const char * p, * q;
	int l;

	if(!path || !prefix || !result)
    {
		return NX_False;
    }

	if(*path == '/')
    {
		path++;
    }

	if(*prefix == '/')
    {
		prefix++;
    }

	l = NX_StrLen(prefix);
	if(NX_StrCmpN(path, prefix, l) != 0)
    {
		return NX_False;
    }

	p = &path[l];
	if(*p == '\0')
    {
		return NX_False;
    }
	if(*p == '/')
    {
		p++;
    }
	if(*p == '\0')
    {
		return NX_False;
    }

	q = NX_StrChr(p, '/');
	if(q)
	{
		if (*(q + 1) != '\0')
        {
			return NX_False;
        }
		l = q - p;
	}
	else
	{
		l = NX_StrLen(p);
	}
	NX_MemCopy(result, p, l);
	result[l] = '\0';

	return NX_True;
}

NX_PRIVATE NX_Bool CheckPath(const char * path, const char * prefix, const char * name)
{
	int l;

	if(!path || !prefix || !name)
    {
		return NX_False;
    }

	if(path[0] == '/')
    {
		path++;
    }

	if(prefix[0] == '/')
    {
		prefix++;
    }

	l = NX_StrLen(prefix);
	if(l && (NX_StrCmpN(path, prefix, l) != 0))
    {
		return NX_False;
    }

	path += l;

	if(path[0] == '/')
    {
		path++;
    }

	if(NX_StrCmp(path, name) != 0)
    {
		return NX_False;
    }

	return NX_True;
}

NX_PRIVATE NX_Error CpioMount(NX_VfsMount * m, const char * dev)
{
	struct CpioNewcHeader header;
	NX_U64 rd;

	if(dev == NX_NULL)
    {
		return NX_EINVAL;
    }

	if(NX_IoBlockCapacity(m->dev) <= sizeof(struct CpioNewcHeader))
    {
		return NX_ENORES;
    }
    
	rd = NX_IoBlockRead(m->dev, (NX_U8 *)(&header), 0, sizeof(struct CpioNewcHeader));
	if(rd != sizeof(struct CpioNewcHeader))
    {
		return NX_ENORES;
    }

	if(NX_StrCmpN((const char *)header.c_magic, "070701", 6) != 0)
    {
		return NX_EFAULT;
    }

	m->flags |= NX_VFS_MOUNT_RO;
	m->root->data = NX_NULL;
	m->data = NX_NULL;

	return NX_EOK;
}

NX_PRIVATE NX_Error CpioUnmount(NX_VfsMount * m)
{
	m->data = NX_NULL;
	return NX_EOK;
}

NX_PRIVATE NX_Error CpioMountSync(NX_VfsMount * m)
{
	return NX_EOK;
}

NX_PRIVATE NX_Error CpioVget(NX_VfsMount * m, NX_VfsNode * n)
{
	return NX_EOK;
}

NX_PRIVATE NX_Error CpioVput(NX_VfsMount * m, NX_VfsNode * n)
{
	return NX_EOK;
}

NX_PRIVATE NX_U64 CpioRead(NX_VfsNode * n, NX_I64 off, void * buf, NX_U64 len, NX_Error *outErr)
{
	NX_U64 toff;
	NX_U64 sz = 0;

	if(n->type != NX_VFS_NODE_TYPE_REG)
    {
        NX_ErrorSet(outErr, NX_EINVAL);
		return 0;
    }

	if(off >= n->size)
    {
        NX_ErrorSet(outErr, NX_EINVAL);
		return 0;
    }

	sz = len;
	if((n->size - off) < sz)
    {
		sz = n->size - off;
    }

	toff = (NX_U64)((unsigned long)(n->data));
	sz = NX_IoBlockRead(n->mount->dev, (NX_U8 *)buf, (toff + off), sz);

    NX_ErrorSet(outErr, NX_EOK);
	return sz;
}

NX_PRIVATE NX_U64 CpioWrite(NX_VfsNode * n, NX_I64 off, void * buf, NX_U64 len, NX_Error *outErr)
{
    NX_ErrorSet(outErr, NX_EPERM);
	return 0;
}

NX_PRIVATE NX_Error CpioTruncate(NX_VfsNode * n, NX_I64 off)
{
	return NX_EPERM; /* can't truncate */
}

NX_PRIVATE NX_Error CpioSync(NX_VfsNode * n)
{
	return NX_EOK;
}

NX_PRIVATE NX_Error CpioReaddir(NX_VfsNode * dn, NX_I64 off, NX_VfsDirent * d)
{
	struct CpioNewcHeader header;
	char path[NX_VFS_MAX_PATH];
	char name[NX_VFS_MAX_NAME];
	NX_U32 size, name_size, mode;
	NX_U64 toff = 0, rd;
	char buf[9];
	int i = 0;

	while(1)
	{
		rd = NX_IoBlockRead(dn->mount->dev, (NX_U8 *)&header, toff, sizeof(struct CpioNewcHeader));
		if(rd != sizeof(struct CpioNewcHeader))
        {
			return NX_ENORES;
        }

		if(NX_StrCmpN((const char *)&header.c_magic, "070701", 6) != 0)
        {
			return NX_ENORES;
        }

		buf[8] = '\0';

		NX_MemCopy(buf, &header.c_filesize, 8);
		size = NX_StrToUL((const char *)buf, NX_NULL, 16);

		NX_MemCopy(buf, &header.c_namesize, 8);
		name_size = NX_StrToUL((const char *)buf, NX_NULL, 16);

		NX_MemCopy(buf, &header.c_mode, 8);
		mode = NX_StrToUL((const char *)buf, NX_NULL, 16);

		rd = NX_IoBlockRead(dn->mount->dev, (NX_U8 *)path, toff + sizeof(struct CpioNewcHeader), name_size);
		if(!rd)
        {
			return NX_EIO;
        }

		if((size == 0) && (mode == 0) && (name_size == 11) && (NX_StrCmpN(path, "TRAILER!!!", 10) == 0))
		{
        	return NX_EIO;
        }

		toff += sizeof(struct CpioNewcHeader);
		toff += (((name_size + 1) & ~3) + 2) + size;
		toff = (toff + 3) & ~3;

		if(path[0] == '.')
        {
			continue;
        }

		if(!GetNextToken(path, dn->path, name))
        {
			continue;
        }

		if(i++ == off)
		{
			toff = 0;
			break;
		}
	}

	if((mode & 00170000) == 0140000)
	{
		d->type = NX_VFS_DIR_TYPE_SOCK;
	}
	else if((mode & 00170000) == 0120000)
	{
		d->type = NX_VFS_DIR_TYPE_LNK;
	}
	else if ((mode & 00170000) == 0100000)
	{
		d->type = NX_VFS_DIR_TYPE_REG;
	}
	else if ((mode & 00170000) == 0060000)
	{
		d->type = NX_VFS_DIR_TYPE_BLK;
	}
	else if ((mode & 00170000) == 0040000)
	{
		d->type = NX_VFS_DIR_TYPE_DIR;
	}
	else if ((mode & 00170000) == 0020000)
	{
		d->type = NX_VFS_DIR_TYPE_CHR;
	}
	else if ((mode & 00170000) == 0010000)
	{
		d->type = NX_VFS_DIR_TYPE_FIFO;
	}
	else
	{
		d->type = NX_VFS_DIR_TYPE_REG;
	}

	NX_StrCopySafe(d->name, name, sizeof(d->name));
	d->off = off;
	d->reclen = 1;

	return NX_EOK;
}

NX_PRIVATE NX_Error CpioLookup(NX_VfsNode * dn, const char * name, NX_VfsNode * n)
{
	struct CpioNewcHeader header;
	char path[NX_VFS_MAX_PATH];
	NX_U64 off = 0, rd;
	NX_U32 size, name_size, mode, mtime;
	NX_U8 buf[9];

	while(1)
	{
		rd = NX_IoBlockRead(dn->mount->dev, (NX_U8 *)&header, off, sizeof(struct CpioNewcHeader));
		if(rd != sizeof(struct CpioNewcHeader))
        {
			return NX_ENORES;
        }

		if(NX_StrCmpN((const char *)header.c_magic, "070701", 6) != 0)
        {
			return NX_ENORES;
        }

		buf[8] = '\0';

		NX_MemCopy(buf, &header.c_filesize, 8);
		size = NX_StrToUL((const char *)buf, NX_NULL, 16);

		NX_MemCopy(buf, &header.c_namesize, 8);
		name_size = NX_StrToUL((const char *)buf, NX_NULL, 16);

		NX_MemCopy(buf, &header.c_mode, 8);
		mode = NX_StrToUL((const char *)buf, NX_NULL, 16);

		NX_MemCopy(buf, &header.c_mtime, 8);
		mtime = NX_StrToUL((const char *)buf, NX_NULL, 16);

		rd = NX_IoBlockRead(dn->mount->dev, (NX_U8 *)path, off + sizeof(struct CpioNewcHeader), name_size);
		if(!rd)
        {
			return -1;
        }

		if((size == 0) && (mode == 0) && (name_size == 11) && (NX_StrCmpN(path, "TRAILER!!!", 10) == 0))
		{
        	return -1;
        }

		if((path[0] != '.') && CheckPath(path, dn->path, name))
        {
			break;
        }

		off += sizeof(struct CpioNewcHeader);
		off += (((name_size + 1) & ~3) + 2) + size;
		off = (off + 3) & ~0x3;
	}

	n->atime = mtime;
	n->mtime = mtime;
	n->ctime = mtime;
	n->mode = 0;

	if((mode & 00170000) == 0140000)
	{
		n->type = NX_VFS_NODE_TYPE_SOCK;
		n->mode |= NX_VFS_S_IFSOCK;
	}
	else if((mode & 00170000) == 0120000)
	{
		n->type = NX_VFS_NODE_TYPE_LNK;
		n->mode |= NX_VFS_S_IFLNK;
	}
	else if((mode & 00170000) == 0100000)
	{
		n->type = NX_VFS_NODE_TYPE_REG;
		n->mode |= NX_VFS_S_IFREG;
	}
	else if((mode & 00170000) == 0060000)
	{
		n->type = NX_VFS_NODE_TYPE_BLK;
		n->mode |= NX_VFS_S_IFBLK;
	}
	else if((mode & 00170000) == 0040000)
	{
		n->type = NX_VFS_NODE_TYPE_DIR;
		n->mode |= NX_VFS_S_IFDIR;
	}
	else if((mode & 00170000) == 0020000)
	{
		n->type = NX_VFS_NODE_TYPE_CHR;
		n->mode |= NX_VFS_S_IFCHR;
	}
	else if((mode & 00170000) == 0010000)
	{
		n->type = NX_VFS_NODE_TYPE_FIFO;
		n->mode |= NX_VFS_S_IFIFO;
	}
	else
	{
		n->type = NX_VFS_NODE_TYPE_REG;
	}

	n->mode |= (mode & 00400) ? NX_VFS_S_IRUSR : 0;
	n->mode |= (mode & 00200) ? NX_VFS_S_IWUSR : 0;
	n->mode |= (mode & 00100) ? NX_VFS_S_IXUSR : 0;
	n->mode |= (mode & 00040) ? NX_VFS_S_IRGRP : 0;
	n->mode |= (mode & 00020) ? NX_VFS_S_IWGRP : 0;
	n->mode |= (mode & 00010) ? NX_VFS_S_IXGRP : 0;
	n->mode |= (mode & 00004) ? NX_VFS_S_IROTH : 0;
	n->mode |= (mode & 00002) ? NX_VFS_S_IWOTH : 0;
	n->mode |= (mode & 00001) ? NX_VFS_S_IXOTH : 0;
	n->size = size;

	off += sizeof(struct CpioNewcHeader);
	off += (((name_size + 1) & ~3) + 2);
	n->data = (void *)((unsigned long)off);

	return NX_EOK;
}

NX_PRIVATE NX_Error CpioCreate(NX_VfsNode * dn, const char * filename, NX_U32 mode)
{
	return NX_EPERM;
}

NX_PRIVATE NX_Error CpioRemove(NX_VfsNode * dn, NX_VfsNode * n, const char *name)
{
	return NX_EPERM;
}

NX_PRIVATE NX_Error CpioRename(NX_VfsNode * sn, const char * sname, NX_VfsNode * n, NX_VfsNode * dn, const char * dname)
{
	return NX_EPERM;
}

NX_PRIVATE NX_Error CpioMkdir(NX_VfsNode * dn, const char * name, NX_U32 mode)
{
	return NX_EPERM;
}

NX_PRIVATE NX_Error CpioRmdir(NX_VfsNode * dn, NX_VfsNode * n, const char *name)
{
	return NX_EPERM;
}

NX_PRIVATE NX_Error CpioChmod(NX_VfsNode * n, NX_U32 mode)
{
	return NX_EPERM;
}

NX_PRIVATE NX_VfsFileSystem cpio = {
	.name		= "cpio",

	.mount		= CpioMount,
	.unmount	= CpioUnmount,
	.msync		= CpioMountSync,
	.vget		= CpioVget,
	.vput		= CpioVput,

	.read		= CpioRead,
	.write		= CpioWrite,
	.truncate	= CpioTruncate,
	.sync		= CpioSync,
	.readdir	= CpioReaddir,
	.lookup		= CpioLookup,
	.create		= CpioCreate,
	.remove		= CpioRemove,
	.rename		= CpioRename,
	.mkdir		= CpioMkdir,
	.rmdir		= CpioRmdir,
	.chmod		= CpioChmod,
};

NX_PRIVATE void NX_FileSystemCpioInit(void)
{
	NX_VfsRegisterFileSystem(&cpio);
}

NX_PRIVATE void NX_FileSystemCpioExit(void)
{
	NX_VfsUnregisterFileSystem(&cpio);
}

NX_FS_INIT(NX_FileSystemCpioInit);
NX_FS_EXIT(NX_FileSystemCpioExit);
