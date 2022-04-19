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
#include <mm/alloc.h>
#include <xbook/init_call.h>
#include <utils/log.h>
#include <io/block.h>
#include <sched/thread.h>

#define VFS_GET_FILE_TABLE() NX_ThreadGetFileTable(NX_ThreadSelf())

NX_PRIVATE NX_LIST_HEAD(FileSystemListHead);

NX_PRIVATE NX_SPIN_DEFINE_UNLOCKED(FileSystemLock);

NX_PRIVATE NX_List NX_FileSystemMountList;
NX_PRIVATE NX_Mutex NX_FileSystemMountLock;
NX_PRIVATE NX_List NX_FileSystemNodeList[NX_VFS_NODE_HASH_SIZE];
NX_PRIVATE NX_Mutex NX_FileSystemNodeLock[NX_VFS_NODE_HASH_SIZE];

/* default file table for kernel thread */
NX_PRIVATE NX_VfsFileTable DefaultFileTable;

NX_Error NX_VfsRegisterFileSystem(NX_VfsFileSystem * fs)
{
    NX_UArch level;
	if(!fs || !fs->name)
    {
		return NX_EINVAL;
    }

	NX_SpinLockIRQ(&FileSystemLock, &level);
	NX_ListAddTail(&fs->list, &FileSystemListHead);
    NX_SpinUnlockIRQ(&FileSystemLock, level);
	return NX_EOK;
}

NX_Error NX_VfsUnregisterFileSystem(NX_VfsFileSystem * fs)
{
	NX_UArch level;

	if(!fs || !fs->name)
    {
		return NX_EINVAL;
    }

	NX_SpinLockIRQ(&FileSystemLock, &level);
	NX_ListDel(&fs->list);
	NX_SpinUnlockIRQ(&FileSystemLock, level);

	return NX_EOK;
}

NX_VfsFileSystem * NX_VfsSearchFileSystem(const char * name)
{
    NX_VfsFileSystem * pos, * n;

	if(!name)
    {
		return NX_NULL;        
    }

	NX_ListForEachEntrySafe(pos, n, &FileSystemListHead, list)
	{
		if(NX_StrCmp(pos->name, name) == 0)
        {
			return pos;
        }
	}
	return NX_NULL;
}

NX_VfsFileTable *NX_VfsGetDefaultFileTable(void)
{
    return &DefaultFileTable;
}

NX_PRIVATE int CountMatch(const char * path, char * mount_root)
{
	int len = 0;

	while(*path && *mount_root)
	{
		if(*path != *mount_root)
        {
			break;
        }
		path++;
		mount_root++;
		len++;
	}

	if(*mount_root != '\0')
    {
		return 0;
    }

	if((len == 1) && (*(path - 1) == '/'))
    {
		return 1;
    }

	if((*path == '\0') || (*path == '/'))
    {
		return len;
    }

	return 0;
}

NX_PRIVATE NX_Error VfsFindRoot(const char * path, NX_VfsMount ** mp, char ** root)
{
	NX_VfsMount * pos, * m = NX_NULL;
	int len, max_len = 0;

	if(!path || !mp || !root)
    {
		return NX_EINVAL;
    }

	NX_MutexLock(&NX_FileSystemMountLock);
	NX_ListForEachEntry(pos, &NX_FileSystemMountList, link)
	{
		len = CountMatch(path, pos->path);
		if(len > max_len)
		{
			max_len = len;
			m = pos;
		}
	}
	NX_MutexUnlock(&NX_FileSystemMountLock);

	if(!m)
    {
		return NX_ENOSRCH;
    }

	*root = (char *)(path + max_len);
	while(**root == '/')
	{
		(*root)++;
	}
	*mp = m;

	return NX_EOK;
}

void NX_VfsFileTableInit(NX_VfsFileTable *ft)
{
    int i;
	for(i = 0; i < NX_VFS_MAX_FD; i++)
	{
		NX_MutexInit(&ft->file[i].lock);
		ft->file[i].node = NX_NULL;
		ft->file[i].offset = 0;
		ft->file[i].flags = 0;
	}
	NX_MutexInit(&ft->file_lock);
}

NX_PRIVATE int VfsFileAlloc(NX_VfsFileTable *ft)
{
	int i, fd = -1;

	NX_MutexLock(&ft->file_lock);
	for(i = 3; i < NX_VFS_MAX_FD; i++)
	{
		if(ft->file[i].node == NX_NULL)
		{
			fd = i;
			break;
		}
	}
	NX_MutexUnlock(&ft->file_lock);

	return fd;
}

NX_PRIVATE void VfsFileFree(NX_VfsFileTable *ft, int fd)
{
	if((fd >= 3) && (fd < NX_VFS_MAX_FD))
	{
		NX_MutexLock(&ft->file_lock);
		if(ft->file[fd].node)
		{
			NX_MutexLock(&ft->file[fd].lock);
			ft->file[fd].node = NX_NULL;
			ft->file[fd].offset = 0;
			ft->file[fd].flags = 0;
			NX_MutexUnlock(&ft->file[fd].lock);
		}
		NX_MutexUnlock(&ft->file_lock);
	}
}


NX_PRIVATE NX_VfsFile * VfsFileDescriptorToFile(NX_VfsFileTable *ft, int fd)
{
    if (!ft)
    {
        return NX_NULL;
    }
	return ((fd >= 0) && (fd < NX_VFS_MAX_FD)) ? &ft->file[fd] : NX_NULL;
}

NX_PRIVATE NX_U32 VfsNodeHash(NX_VfsMount * m, const char * path)
{
	NX_U32 val = 0;

	if(path)
	{
		while(*path)
        {
			val = ((val << 5) + val) + *path++;
        }
	}
	return (val ^ (NX_U32)((NX_UArch)m)) & (NX_VFS_NODE_HASH_SIZE - 1);
}

NX_PRIVATE NX_VfsNode * VfsNodeGet(NX_VfsMount * m, const char * path)
{
	NX_VfsNode * n;
	NX_U32 hash = VfsNodeHash(m, path);
	int err;

	if(!(n = NX_MemAlloc(sizeof(NX_VfsNode))))
    {
		return NX_NULL;
    }

	NX_ListInit(&n->link);
	NX_MutexInit(&n->lock);
	n->mount = m;
	NX_AtomicSet(&n->refcnt, 1);
	if(NX_StrCopySafe(n->path, path, sizeof(n->path)) >= sizeof(n->path))
	{
		NX_MemFree(n);
		return NX_NULL;
	}

	NX_MutexLock(&m->lock);
	err = m->fs->vget(m, n);
	NX_MutexUnlock(&m->lock);
	if(err != NX_EOK)
	{
		NX_MemFree(n);
		return NX_NULL;
	}

	NX_AtomicAdd(&m->refcnt, 1);
	NX_MutexLock(&NX_FileSystemNodeLock[hash]);
	NX_ListAdd(&n->link, &NX_FileSystemNodeList[hash]);
	NX_MutexUnlock(&NX_FileSystemNodeLock[hash]);

	return n;
}

NX_PRIVATE NX_VfsNode * VfsNodeLookup(NX_VfsMount * m, const char * path)
{
	NX_VfsNode * n;
	NX_U32 hash = VfsNodeHash(m, path);
	int found = 0;

	NX_MutexLock(&NX_FileSystemNodeLock[hash]);
	NX_ListForEachEntry(n, &NX_FileSystemNodeList[hash], link)
	{
		if((n->mount == m) && (!NX_StrCmpN(n->path, path, NX_VFS_MAX_PATH)))
		{
			found = 1;
			break;
		}
	}
	NX_MutexUnlock(&NX_FileSystemNodeLock[hash]);

	if(!found)
    {
		return NX_NULL;
    }
	NX_AtomicAdd(&n->refcnt, 1);

	return n;
}

NX_PRIVATE void VfsNodeIncreaseRef(NX_VfsNode * n)
{
	NX_AtomicAdd(&n->refcnt, 1);
}

NX_PRIVATE void VfsNodePut(NX_VfsNode * n)
{
	NX_U32 hash;

    /* FIXME: add lock for sub and get */
    NX_AtomicSub(&n->refcnt, 1);
	if(NX_AtomicGet(&n->refcnt))
    {
		return;
    }

	hash = VfsNodeHash(n->mount, n->path);
	NX_MutexLock(&NX_FileSystemNodeLock[hash]);
	NX_ListDel(&n->link);
	NX_MutexUnlock(&NX_FileSystemNodeLock[hash]);

	NX_MutexLock(&n->mount->lock);
	n->mount->fs->vput(n->mount, n);
	NX_MutexUnlock(&n->mount->lock);

	NX_AtomicSub(&n->mount->refcnt, 1);
	NX_MemFree(n);
}

NX_PRIVATE NX_Error VfsNodeStat(NX_VfsNode * n, NX_VfsStatInfo * st)
{
	NX_U32 mode;

	NX_MemZero(st, sizeof(NX_VfsStatInfo));

	st->ino = (NX_U64)((NX_UArch)n);
	NX_MutexLock(&n->lock);
	st->size = n->size;
	mode = n->mode & (NX_VFS_S_IRWXU | NX_VFS_S_IRWXG | NX_VFS_S_IRWXO);
	st->ctime = n->ctime;
	st->atime = n->atime;
	st->mtime = n->mtime;
	NX_MutexUnlock(&n->lock);

	switch(n->type)
	{
	case NX_VFS_NODE_TYPE_REG:
		mode |= NX_VFS_S_IFREG;
		break;
	case NX_VFS_NODE_TYPE_DIR:
		mode |= NX_VFS_S_IFDIR;
		break;
	case NX_VFS_NODE_TYPE_BLK:
		mode |= NX_VFS_S_IFBLK;
		break;
	case NX_VFS_NODE_TYPE_CHR:
		mode |= NX_VFS_S_IFCHR;
		break;
	case NX_VFS_NODE_TYPE_LNK:
		mode |= NX_VFS_S_IFLNK;
		break;
	case NX_VFS_NODE_TYPE_SOCK:
		mode |= NX_VFS_S_IFSOCK;
		break;
	case NX_VFS_NODE_TYPE_FIFO:
		mode |= NX_VFS_S_IFIFO;
		break;
	default:
		return NX_ENORES;
	};
	st->mode = mode;

	if(n->type == NX_VFS_NODE_TYPE_CHR || n->type == NX_VFS_NODE_TYPE_BLK)
		st->dev = (NX_U64)((NX_UArch)n->data);
	st->uid = 0;
	st->gid = 0;

	return NX_EOK;
}

NX_PRIVATE int VfsNodeAccess(NX_VfsNode * n, NX_U32 mode)
{
	NX_U32 m;

	NX_MutexLock(&n->lock);
	m = n->mode;
	NX_MutexUnlock(&n->lock);

	if((mode & NX_VFS_R_OK) && !(m & (NX_VFS_S_IRUSR | NX_VFS_S_IRGRP | NX_VFS_S_IROTH)))
    {
		return -1;
    }

	if(mode & NX_VFS_W_OK)
	{
		if(n->mount->flags & NX_VFS_MOUNT_RO)
        {
			return -1;
        }

		if(!(m & (NX_VFS_S_IWUSR | NX_VFS_S_IWGRP | NX_VFS_S_IWOTH)))
		{
        	return -1;
        }
	}

	if((mode & NX_VFS_X_OK) && !(m & (NX_VFS_S_IXUSR | NX_VFS_S_IXGRP | NX_VFS_S_IXOTH)))
    {
		return -1;
    }

	return 0;
}

NX_PRIVATE void VfsNodeRelease(NX_VfsNode * n)
{
	NX_VfsMount * m;
	char path[NX_VFS_MAX_PATH];
	char * p;

	if(!n)
    {
		return;
    }

	m = n->mount;

	if(m->root == n)
	{
		VfsNodePut(n);
		return;
	}

	if(NX_StrCopySafe(path, n->path, sizeof(path)) >= sizeof(path))
    {
		return;
    }

	VfsNodePut(n);

	while(1)
	{
		p = NX_StrChrReverse(path, '/');
		if(!p)
        {
			break;
        }
		*p = '\0';

		if(path[0] == '\0')
        {
			break;
        }

		n = VfsNodeLookup(m, path);
		if(!n)
        {
			continue;
        }

		VfsNodePut(n);
		VfsNodePut(n);
	}
	VfsNodePut(m->root);
}

NX_PRIVATE int VfsNodeAcquire(const char * path, NX_VfsNode ** np)
{
	NX_VfsMount * m;
	NX_VfsNode * dn, * n;
	char node[NX_VFS_MAX_PATH];
	char * p;
	NX_Error err;
    int i, j;

	if(VfsFindRoot(path, &m, &p) != NX_EOK)
    {
		return -1;
    }

	if(!m->root)
    {
		return -1;
    }

	dn = n = m->root;
	VfsNodeIncreaseRef(dn);

	i = 0;
	while(*p != '\0')
	{
		while(*p == '/')
        {
			p++;
        }

		if(*p == '\0')
        {
			break;
        }

		node[i] = '/';
		i++;
		j = i;
		while(*p != '\0' && *p != '/')
		{
			node[i] = *p;
			p++;
			i++;
		}
		node[i] = '\0';

		n = VfsNodeLookup(m, node);
		if(n == NX_NULL)
		{
			n = VfsNodeGet(m, node);
			if(n == NX_NULL)
			{
				VfsNodePut(dn);
				return -1;
			}

			NX_MutexLock(&n->lock);
			NX_MutexLock(&dn->lock);
			err = dn->mount->fs->lookup(dn, &node[j], n);
			NX_MutexUnlock(&dn->lock);
			NX_MutexUnlock(&n->lock);
			if(err != NX_EOK || (*p == '/' && n->type != NX_VFS_NODE_TYPE_DIR))
			{
				VfsNodeRelease(n);
				return -1;
			}
		}
		dn = n;
	}
	*np = n;

	return 0;
}

NX_Error NX_VfsMountFileSystem(const char * dev, const char * dir, const char * fsname, NX_U32 flags)
{
	NX_Device * bdev;
	NX_VfsFileSystem * fs;
	NX_VfsMount * m, * tm;
	NX_VfsNode * n, * n_covered;
	int err;

	if(!dir || *dir == '\0')
    {
		return NX_EINVAL;
    }

	if(!(fs = NX_VfsSearchFileSystem(fsname)))
    {
		return NX_ENOSRCH;
    }

	if(dev != NX_NULL)
	{
        if(!(bdev = NX_IoBlockOpen(dev)))
        {
			return NX_ENOSRCH;
        }
	}
	else
	{
		bdev = NX_NULL;
	}

	if(!(m = NX_MemAlloc(sizeof(NX_VfsMount))))
    {
		return NX_ENOMEM;
    }

	NX_ListInit(&m->link);
	NX_MutexInit(&m->lock);
	m->fs = fs;
	m->flags = flags & NX_VFS_MOUNT_MASK;
	NX_AtomicSet(&m->refcnt, 0);
	if(NX_StrCopySafe(m->path, dir, sizeof(m->path)) >= sizeof(m->path))
	{
		NX_MemFree(m);
		return NX_EFAULT;
	}
	m->dev = bdev;

	if(*dir == '/' && *(dir + 1) == '\0')
	{
		n_covered = NX_NULL;
	}
	else
	{
		if(VfsNodeAcquire(dir, &n_covered) != 0)
		{
			NX_MemFree(m);
			return NX_ENORES;
		}
		if(n_covered->type != NX_VFS_NODE_TYPE_DIR)
		{
			VfsNodeRelease(n_covered);
			NX_MemFree(m);
			return NX_EFAULT;
		}
	}
	m->covered = n_covered;

	if(!(n = VfsNodeGet(m, "/")))
	{
		if(m->covered)
        {
			VfsNodeRelease(m->covered);
        }
		NX_MemFree(m);
		return NX_EFAULT;
	}
	n->type = NX_VFS_NODE_TYPE_DIR;
	n->flags = NX_VFS_NODE_FLAG_ROOT;
	n->mode = NX_VFS_S_IFDIR | NX_VFS_S_IRWXU | NX_VFS_S_IRWXG | NX_VFS_S_IRWXO;
	m->root = n;

	NX_MutexLock(&m->lock);
	err = m->fs->mount(m, dev);
	NX_MutexUnlock(&m->lock);
	if(err != NX_EOK)
	{
		VfsNodeRelease(m->root);
		if(m->covered)
        {
			VfsNodeRelease(m->covered);
        }
		NX_MemFree(m);
		return err;
	}

	if(m->flags & NX_VFS_MOUNT_RO)
    {
		m->root->mode &= ~(NX_VFS_S_IWUSR | NX_VFS_S_IWGRP | NX_VFS_S_IWOTH);
    }

	NX_MutexLock(&NX_FileSystemMountLock);
	NX_ListForEachEntry(tm, &NX_FileSystemMountList, link)
	{
		if(!NX_StrCmp(tm->path, dir) || ((dev != NX_NULL) && (tm->dev == bdev)))
		{
			NX_MutexUnlock(&NX_FileSystemMountLock);
			NX_MutexLock(&m->lock);
			m->fs->unmount(m);
			NX_MutexUnlock(&m->lock);
			VfsNodeRelease(m->root);
			if(m->covered)
            {
				VfsNodeRelease(m->covered);
            }
			NX_MemFree(m);
			return NX_EAGAIN;
		}
	}
	NX_ListAdd(&m->link, &NX_FileSystemMountList);
	NX_MutexUnlock(&NX_FileSystemMountLock);

	return NX_EOK;
}

NX_Error NX_VfsUnmountFileSystem(const char * path)
{
	NX_VfsMount * m;
	int found;
	NX_Error err;

	NX_MutexLock(&NX_FileSystemMountLock);
	found = 0;
	NX_ListForEachEntry(m, &NX_FileSystemMountList, link)
	{
		if(!NX_StrCmp(path, m->path))
		{
			found = 1;
			break;
		}
	}
	if(!found)
	{
		NX_MutexUnlock(&NX_FileSystemMountLock);
		return NX_ENOSRCH;
	}
	if(NX_AtomicGet(&m->refcnt) > 1)
	{
		NX_MutexUnlock(&NX_FileSystemMountLock);
		return NX_EBUSY;
	}
	NX_ListDel(&m->link);
	NX_MutexUnlock(&NX_FileSystemMountLock);

	NX_MutexLock(&m->lock);
	err = m->fs->msync(m);
	m->fs->unmount(m);
	NX_MutexUnlock(&m->lock);

	VfsNodeRelease(m->root);
	if(m->covered)
    {
		VfsNodeRelease(m->covered);
    }
	
    if(m->dev)
    {
		NX_IoBlockSync(m->dev);
        NX_IoBlockClose(m->dev);
    }
	
    NX_MemFree(m);

	return err;
}

NX_Error NX_VfsSync(void)
{
	NX_VfsMount * m;
    NX_Error err;
	NX_MutexLock(&NX_FileSystemMountLock);
	NX_ListForEachEntry(m, &NX_FileSystemMountList, link)
	{
		NX_MutexLock(&m->lock);
		err = m->fs->msync(m);
		if (err != NX_EOK)
        {
            NX_LOG_W("mount point path %s sync error! with %d", m->path, err);
        }
        NX_MutexUnlock(&m->lock);
	}
	NX_MutexUnlock(&NX_FileSystemMountLock);

	return NX_EOK;
}

NX_VfsMount * NX_VfsGetMount(int index)
{
	NX_VfsMount * m = NX_NULL;
	int found = 0;

	if(index < 0)
		return NX_NULL;

	NX_MutexLock(&NX_FileSystemMountLock);
	NX_ListForEachEntry(m, &NX_FileSystemMountList, link)
	{
		if(!index)
		{
			found = 1;
			break;
		}
		index--;
	}
	NX_MutexUnlock(&NX_FileSystemMountLock);

	if(!found)
    {
		return NX_NULL;
    }
	return m;
}

int NX_VfsGetMountCount(void)
{
	NX_VfsMount * m;
	int ret = 0;

	NX_MutexLock(&NX_FileSystemMountLock);
	NX_ListForEachEntry(m, &NX_FileSystemMountList, link)
	{
		ret++;
	}
	NX_MutexUnlock(&NX_FileSystemMountLock);

	return ret;
}

NX_PRIVATE int VfsLookupDir(const char * path, NX_VfsNode ** np, char ** name)
{
	NX_VfsNode * n;
	char buf[NX_VFS_MAX_PATH];
	char * file, * dir;
	int err;

	if(NX_StrCopySafe(buf, path, sizeof(buf)) >= sizeof(buf))
    {
		return -1;
    }

	file = NX_StrChrReverse(buf, '/');
	if(!file)
    {
		return -1;
    }

	if(!buf[0])
    {
		return -1;
    }

	if(file == buf)
	{
		dir = "/";
	}
	else
	{
		*file = '\0';
		dir = buf;
	}

	if((err = VfsNodeAcquire(dir, &n)))
    {
		return err;
    }
	if(n->type != NX_VFS_NODE_TYPE_DIR)
	{
		VfsNodeRelease(n);
		return -1;
	}
	*np = n;

	*name = NX_StrChrReverse(path, '/');
	if(*name == NX_NULL)
    {
		return -1;
    }
	*name += 1;

	return 0;
}

int NX_VfsOpen(const char * path, NX_U32 flags, NX_U32 mode, NX_Error *outErr)
{
	NX_VfsNode * n, * dn;
	NX_VfsFile * f;
	char * filename;
	int err, fd;
    NX_VfsFileTable *ft;

	if(!path || !(flags & NX_VFS_O_ACCMODE))
    {
        NX_ErrorSet(outErr, NX_EINVAL);
		return -1;
    }

    /* check has file table */
    ft = VFS_GET_FILE_TABLE();
    if (!ft)
    {
        NX_ErrorSet(outErr, NX_ENORES);
        return -1;
    }

	if(flags & NX_VFS_O_CREAT)
	{
		err = VfsNodeAcquire(path, &n);
		if(err)
		{
			if((err = VfsLookupDir(path, &dn, &filename)))
            {
                NX_ErrorSet(outErr, NX_ENOSRCH);
				return err;
            }
			if((err = VfsNodeAccess(dn, NX_VFS_W_OK)))
			{
                NX_ErrorSet(outErr, NX_EPERM);
				VfsNodeRelease(dn);
				return err;
			}
			mode &= ~NX_VFS_S_IFMT;
			mode |= NX_VFS_S_IFREG;
			NX_MutexLock(&dn->lock);
			err = dn->mount->fs->create(dn, filename, mode);
			if(err == NX_EOK)
            {
				err = dn->mount->fs->sync(dn);
            }
			NX_MutexUnlock(&dn->lock);
			VfsNodeRelease(dn);
			if(err != NX_EOK)
            {
                NX_ErrorSet(outErr, err);
				return -1;
            }
			if((err = VfsNodeAcquire(path, &n)))
            {
                NX_ErrorSet(outErr, NX_ENORES);
				return err;
            }
			flags &= ~NX_VFS_O_TRUNC;
		}
		else
		{
			if(flags & NX_VFS_O_EXCL)
			{
                NX_ErrorSet(outErr, NX_EAGAIN);
				VfsNodeRelease(n);
				return -1;
			}
			flags &= ~NX_VFS_O_CREAT;
		}
	}
	else
	{
		if((err = VfsNodeAcquire(path, &n)))
		{
            NX_ErrorSet(outErr, NX_ENORES);
			return err;
		}
		if((flags & NX_VFS_O_WRONLY) || (flags & NX_VFS_O_TRUNC))
		{
			if((err = VfsNodeAccess(n, NX_VFS_W_OK)))
			{
                NX_ErrorSet(outErr, NX_EPERM);
				VfsNodeRelease(n);
				return err;
			}
			if(n->type == NX_VFS_NODE_TYPE_DIR)
			{
                NX_ErrorSet(outErr, NX_EPERM);
				VfsNodeRelease(n);
				return -1;
			}
		}
	}

	if(flags & NX_VFS_O_TRUNC)
	{
		if(!(flags & NX_VFS_O_WRONLY) || (n->type == NX_VFS_NODE_TYPE_DIR))
		{
            NX_ErrorSet(outErr, NX_EPERM);
			VfsNodeRelease(n);
			return -1;
		}
		NX_MutexLock(&n->lock);
		err = n->mount->fs->truncate(n, 0);
		NX_MutexUnlock(&n->lock);
		if(err)
		{
            NX_ErrorSet(outErr, err);
			VfsNodeRelease(n);
			return -1;
		}
	}

    fd = VfsFileAlloc(ft);
	if(fd < 0)
	{
        NX_ErrorSet(outErr, NX_ENORES);
		VfsNodeRelease(n);
		return -1;
	}
	f = VfsFileDescriptorToFile(ft, fd);

	NX_MutexLock(&f->lock);
	f->node = n;
	f->offset = 0;
	f->flags = flags;
	NX_MutexUnlock(&f->lock);

    NX_ErrorSet(outErr, NX_EOK);
	return fd;
}

NX_Error NX_VfsClose(int fd)
{
	NX_VfsNode * n;
	NX_VfsFile * f;
	NX_Error err;
    NX_VfsFileTable *ft;

    /* check has file table */
    ft = VFS_GET_FILE_TABLE();
    if (!ft)
    {
        return NX_ENORES;
    }

	f = VfsFileDescriptorToFile(ft, fd);
	if(!f)
    {
		return NX_EFAULT;
    }

	NX_MutexLock(&f->lock);
	n = f->node;
	if(!n)
	{
		NX_MutexUnlock(&f->lock);
		return NX_EFAULT;
	}

	NX_MutexLock(&n->lock);
	err = n->mount->fs->sync(n);
	NX_MutexUnlock(&n->lock);
	if(err)
	{
		NX_MutexUnlock(&f->lock);
		return err;
	}
	VfsNodeRelease(n);
	NX_MutexUnlock(&f->lock);

	VfsFileFree(ft, fd);
	return NX_EOK;
}

NX_U64 NX_VfsRead(int fd, void * buf, NX_U64 len, NX_Error *outErr)
{
	NX_VfsNode * n;
	NX_VfsFile * f;
	NX_U64 ret;
    NX_VfsFileTable *ft;
    NX_Error err = NX_EOK;

	if(!buf || !len)
    {
        NX_ErrorSet(outErr, NX_EINVAL);
		return 0;
    }

    ft = VFS_GET_FILE_TABLE();
	f = VfsFileDescriptorToFile(ft, fd);
	if(!f)
    {
        NX_ErrorSet(outErr, NX_ENORES);
		return 0;
    }

	NX_MutexLock(&f->lock);
	n = f->node;
	if(!n)
	{
		NX_MutexUnlock(&f->lock);
        NX_ErrorSet(outErr, NX_EFAULT);
		return 0;
	}
	if(n->type != NX_VFS_NODE_TYPE_REG)
	{
		NX_MutexUnlock(&f->lock);
        NX_ErrorSet(outErr, NX_ENORES);
		return 0;
	}

	if(!(f->flags & NX_VFS_O_RDONLY))
	{
		NX_MutexUnlock(&f->lock);
        NX_ErrorSet(outErr, NX_EPERM);
		return 0;
	}

	NX_MutexLock(&n->lock);
	ret = n->mount->fs->read(n, f->offset, buf, len, &err);
	NX_MutexUnlock(&n->lock);
	f->offset += ret;
	NX_MutexUnlock(&f->lock);

    NX_ErrorSet(outErr, err);
	return ret;
}

NX_U64 NX_VfsWrite(int fd, void * buf, NX_U64 len, NX_Error *outErr)
{
	NX_VfsNode * n;
	NX_VfsFile * f;
	NX_U64 ret;
    NX_VfsFileTable *ft;
    NX_Error err = NX_EOK;

	if(!buf || !len)
    {
        NX_ErrorSet(outErr, NX_EINVAL);
		return 0;
    }

    ft = VFS_GET_FILE_TABLE();
	f = VfsFileDescriptorToFile(ft, fd);
	if(!f)
    {
        NX_ErrorSet(outErr, NX_ENORES);
		return 0;
    }

	NX_MutexLock(&f->lock);
	n = f->node;
	if(!n)
	{
		NX_MutexUnlock(&f->lock);
        NX_ErrorSet(outErr, NX_EFAULT);
		return 0;
	}
	if(n->type != NX_VFS_NODE_TYPE_REG)
	{
		NX_MutexUnlock(&f->lock);
        NX_ErrorSet(outErr, NX_ENORES);
		return 0;
	}

	if(!(f->flags & NX_VFS_O_WRONLY))
	{
		NX_MutexUnlock(&f->lock);
        NX_ErrorSet(outErr, NX_EPERM);
		return 0;
	}

	NX_MutexLock(&n->lock);
	ret = n->mount->fs->write(n, f->offset, buf, len, &err);
	NX_MutexUnlock(&n->lock);

	f->offset += ret;
	NX_MutexUnlock(&f->lock);

    NX_ErrorSet(outErr, err);
	return ret;
}

NX_Error NX_VfsIoctl(int fd, NX_U32 cmd, void *arg)
{
	NX_VfsNode * n;
	NX_VfsFile * f;
	NX_Error err;
    NX_VfsFileTable *ft;

    ft = VFS_GET_FILE_TABLE();
	f = VfsFileDescriptorToFile(ft, fd);
	if(!f)
    {
		return NX_ENORES;
    }

	NX_MutexLock(&f->lock);
	n = f->node;
	if(!n)
	{
		NX_MutexUnlock(&f->lock);
		return NX_EFAULT;
	}

	NX_MutexLock(&n->lock);
	err = n->mount->fs->ioctl(n, cmd, arg);
	NX_MutexUnlock(&n->lock);

	NX_MutexUnlock(&f->lock);

	return err;
}

NX_I64 NX_VfsFileSeek(int fd, NX_I64 off, int whence, NX_Error *outErr)
{
	NX_VfsNode * n;
	NX_VfsFile * f;
	NX_I64 ret;
    NX_VfsFileTable *ft;
    
    ft = VFS_GET_FILE_TABLE();
	f = VfsFileDescriptorToFile(ft, fd);
	if(!f)
    {
        NX_ErrorSet(outErr, NX_ENORES);
		return 0;
    }

	NX_MutexLock(&f->lock);
	n = f->node;
	if(!n)
	{
		NX_MutexUnlock(&f->lock);
        NX_ErrorSet(outErr, NX_EPERM);
		return 0;
	}

	NX_MutexLock(&n->lock);
	switch(whence)
	{
	case NX_VFS_SEEK_SET:
		if(off < 0)
			off = 0;
		else if(off > (NX_Offset64)n->size)
			off = n->size;
		break;

	case NX_VFS_SEEK_CUR:
		if((f->offset + off) > (NX_Offset64)n->size)
			off = n->size;
		else if((f->offset + off) < 0)
			off = 0;
		else
			off = f->offset + off;
		break;

	case NX_VFS_SEEK_END:
		if(off > 0)
			off = n->size;
		else if((n->size + off) < 0)
			off = 0;
		else
			off = n->size + off;
		break;

	default:
		NX_MutexUnlock(&n->lock);
		ret = f->offset;
		NX_MutexUnlock(&f->lock);
        NX_ErrorSet(outErr, NX_EINVAL);
		return ret;
	}

	if(off <= n->size)
    {
		f->offset = off;
    }
	NX_MutexUnlock(&n->lock);

	ret = f->offset;
	NX_MutexUnlock(&f->lock);

    NX_ErrorSet(outErr, NX_EOK);
	return ret;
}

NX_Error NX_VfsFileSync(int fd)
{
	NX_VfsNode * n;
	NX_VfsFile * f;
	NX_Error err;
    NX_VfsFileTable *ft;
    
    ft = VFS_GET_FILE_TABLE();
	f = VfsFileDescriptorToFile(ft, fd);
	if(!f)
    {
		return NX_ENORES;
    }

	NX_MutexLock(&f->lock);
	n = f->node;
	if(!n)
	{
		NX_MutexUnlock(&f->lock);
		return NX_EFAULT;
	}
	if(!(f->flags & NX_VFS_O_WRONLY))
	{
		NX_MutexUnlock(&f->lock);
		return NX_EPERM;
	}
	NX_MutexLock(&n->lock);
	err = n->mount->fs->sync(n);
	NX_MutexUnlock(&n->lock);
	NX_MutexUnlock(&f->lock);

	return err;
}

NX_Error NX_VfsFileChmod(int fd, NX_U32 mode)
{
	NX_VfsNode * n;
	NX_VfsFile * f;
	int err;
    NX_VfsFileTable *ft;
    
    ft = VFS_GET_FILE_TABLE();
	f = VfsFileDescriptorToFile(ft, fd);
	if(!f)
    {
		return NX_ENORES;
    }

	NX_MutexLock(&f->lock);
	n = f->node;
	if(!n)
	{
		NX_MutexUnlock(&f->lock);
		return NX_EFAULT;
	}
	mode &= (NX_VFS_S_IRWXU | NX_VFS_S_IRWXG | NX_VFS_S_IRWXO);
	NX_MutexLock(&n->lock);
	err = n->mount->fs->chmod(n, mode);
	NX_MutexUnlock(&n->lock);
	NX_MutexUnlock(&f->lock);

	return err;
}

NX_Error NX_VfsFileStat(int fd, NX_VfsStatInfo * st)
{
	NX_VfsNode * n;
	NX_VfsFile * f;
	NX_Error err;
    NX_VfsFileTable *ft;
    
	if(!st)
    {
		return NX_EINVAL;
    }

    ft = VFS_GET_FILE_TABLE();
	f = VfsFileDescriptorToFile(ft, fd);
	if(!f)
    {
		return NX_ENORES;
    }

	NX_MutexLock(&f->lock);
	n = f->node;
	if(!n)
	{
		NX_MutexUnlock(&f->lock);
		return NX_EFAULT;
	}
	err = VfsNodeStat(n, st);
	NX_MutexUnlock(&f->lock);

	return err;
}

int NX_VfsOpenDir(const char * name, NX_Error *outErr)
{
	NX_VfsNode * n;
	NX_VfsFile * f;
	int fd;
    NX_VfsFileTable *ft;
    NX_Error err = NX_EOK;
    
	if(!name)
    {
        NX_ErrorSet(outErr, NX_EINVAL);
		return -1;
    }

	if((fd = NX_VfsOpen(name, NX_VFS_O_RDONLY, 0, &err)) < 0)
    {
        NX_ErrorSet(outErr, err);
		return fd;
    }

    ft = VFS_GET_FILE_TABLE();
	f = VfsFileDescriptorToFile(ft, fd);
	if(!f)
    {
        NX_ErrorSet(outErr, NX_ENORES);
		return -1;
    }

	NX_MutexLock(&f->lock);
	n = f->node;
	if(!n)
	{
		NX_MutexUnlock(&f->lock);
        NX_ErrorSet(outErr, NX_EFAULT);
		return -1;
	}

	if(n->type != NX_VFS_NODE_TYPE_DIR)
	{
		NX_MutexUnlock(&f->lock);
		NX_VfsClose(fd);
        NX_ErrorSet(outErr, NX_ENORES);
		return -1;
	}
	NX_MutexUnlock(&f->lock);

    NX_ErrorSet(outErr, NX_EOK);
	return fd;
}

NX_Error NX_VfsCloseDir(int fd)
{
	NX_VfsNode * n;
	NX_VfsFile * f;
    NX_VfsFileTable *ft;
    
    ft = VFS_GET_FILE_TABLE();
	f = VfsFileDescriptorToFile(ft, fd);
	if(!f)
    {
		return NX_ENORES;
    }

	NX_MutexLock(&f->lock);
	n = f->node;
	if(!n)
	{
		NX_MutexUnlock(&f->lock);
		return NX_EFAULT;
	}

	if(n->type != NX_VFS_NODE_TYPE_DIR)
	{
		NX_MutexUnlock(&f->lock);
		return NX_ENORES;
	}
	NX_MutexUnlock(&f->lock);

	return NX_VfsClose(fd);
}

NX_Error NX_VfsReadDir(int fd, NX_VfsDirent * dir)
{
	NX_VfsNode * n;
	NX_VfsFile * f;
	NX_Error err;
    NX_VfsFileTable *ft;
    
	if(!dir)
    {
		return NX_EINVAL;
    }

    ft = VFS_GET_FILE_TABLE();
	f = VfsFileDescriptorToFile(ft, fd);
	if(!f)
    {
		return NX_ENORES;
    }

	NX_MutexLock(&f->lock);
	n = f->node;
	if(!n)
	{
		NX_MutexUnlock(&f->lock);
		return NX_EFAULT;
	}
	if(n->type != NX_VFS_NODE_TYPE_DIR)
	{
		NX_MutexUnlock(&f->lock);
		return NX_ENORES;
	}
	NX_MutexLock(&n->lock);
	err = n->mount->fs->readdir(n, f->offset, dir);
	NX_MutexUnlock(&n->lock);
	if(!err)
    {
		f->offset += dir->reclen;
    }
	NX_MutexUnlock(&f->lock);

	return err;
}

NX_Error NX_VfsRewindDir(int fd)
{
	NX_VfsNode * n;
	NX_VfsFile * f;
    NX_VfsFileTable *ft;
    
    ft = VFS_GET_FILE_TABLE();
	f = VfsFileDescriptorToFile(ft, fd);
	if(!f)
    {
		return NX_ENORES;
    }

	NX_MutexLock(&f->lock);
	n = f->node;
	if(!n)
	{
		NX_MutexUnlock(&f->lock);
		return NX_EFAULT;
	}
	if(n->type != NX_VFS_NODE_TYPE_DIR)
	{
		NX_MutexUnlock(&f->lock);
		return NX_ENORES;
	}
	f->offset = 0;
	NX_MutexUnlock(&f->lock);

	return 0;
}

NX_Error NX_VfsMakeDir(const char * path, NX_U32 mode)
{
	NX_VfsNode * n, * dn;
	char * name;
	NX_Error err;

	if(!path)
    {
		return NX_EINVAL;
    }

	if(!VfsNodeAcquire(path, &n))
	{
		VfsNodeRelease(n);
		return NX_EAGAIN;
	}

	if((err = VfsLookupDir(path, &dn, &name)))
    {
		return NX_ENOSRCH;
    }

	if((err = VfsNodeAccess(dn, NX_VFS_W_OK)))
	{
		VfsNodeRelease(dn);
		return NX_EPERM;
	}

	mode &= ~NX_VFS_S_IFMT;
	mode |= NX_VFS_S_IFDIR;

	NX_MutexLock(&dn->lock);

	err = dn->mount->fs->mkdir(dn, name, mode);
	if(err)
    {
		goto fail;
    }
	err = dn->mount->fs->sync(dn);

fail:
	NX_MutexUnlock(&dn->lock);
	VfsNodeRelease(dn);

	return err;
}

NX_PRIVATE NX_Error VfsCheckDirEmpty(const char * path)
{
	NX_VfsDirent dir;
	NX_Error err;
    int fd, count;

	if((fd = NX_VfsOpenDir(path, &err)) < 0)
    {
		return err;
    }

	count = 0;
	do
	{
		err = NX_VfsReadDir(fd, &dir);
		if(err != NX_EOK)
        {
			break;
        }
		if((NX_StrCmp(dir.name, ".") != 0) && (NX_StrCmp(dir.name, "..") != 0))
        {
			count++;
        }
		if(count)
        {
			break;
        }
	} while(1);

	NX_VfsCloseDir(fd);
	if(count)
    {
		return NX_EAGAIN;
    }

	return NX_EOK;
}

NX_Error NX_VfsRemoveDir(const char * path)
{
	NX_VfsNode * n, * dn;
	char * name;
	NX_Error err;

	if(!path)
    {
		return NX_EINVAL;
    }

	if((err = VfsCheckDirEmpty(path)) != NX_EOK)
    {
		return err;
    }

	if(VfsNodeAcquire(path, &n))
    {
		return NX_ENORES;
    }

	if((n->flags == NX_VFS_NODE_FLAG_ROOT) || (NX_AtomicGet(&n->refcnt) >= 2))
	{
		VfsNodeRelease(n);
		return NX_EBUSY;
	}

	if(VfsNodeAccess(n, NX_VFS_W_OK))
	{
		VfsNodeRelease(n);
		return NX_EPERM;
	}

	if(VfsLookupDir(path, &dn, &name))
	{
		VfsNodeRelease(n);
		return NX_ENOSRCH;
	}

	NX_MutexLock(&dn->lock);
	NX_MutexLock(&n->lock);

	err = dn->mount->fs->rmdir(dn, n, name);
	if(err != NX_EOK)
    {
		goto fail;
    }

	err = n->mount->fs->sync(n);
	if(err != NX_EOK)
    {
		goto fail;
    }
	err = dn->mount->fs->sync(dn);

fail:
	NX_MutexUnlock(&n->lock);
	NX_MutexUnlock(&dn->lock);
	VfsNodeRelease(n);
	VfsNodeRelease(dn);

	return err;
}

NX_Error NX_VfsRename(const char * src, const char * dst)
{
	NX_VfsNode * n1, * n2, * sn, * dn;
	char * sname, * dname;
	int len;
    NX_Error err;

	if(!NX_StrCmpN(src, dst, NX_VFS_MAX_PATH))
    {
		return NX_EINVAL;
    }

	if(!NX_StrCmp(src, "/"))
    {
		return NX_EINVAL;
    }

	len = NX_StrLen(src);
	if((len < NX_StrLen(dst)) && !NX_StrCmpN(src, dst, len) && (dst[len] == '/'))
    {
		return NX_EINVAL;
    }

	if(VfsNodeAcquire(src, &n1))
    {
		return NX_ENORES;
    }

	if(VfsNodeAccess(n1, NX_VFS_W_OK))
    {
		err = NX_EPERM;
		goto fail1;
    }

	if(NX_AtomicGet(&n1->refcnt) >= 2)
	{
		err = NX_EBUSY;
		goto fail1;
	}

	if(VfsLookupDir(src, &sn, &sname))
    {
        err = NX_ENOSRCH;
		goto fail1;
    }

	if(!VfsNodeAcquire(dst, &n2))
	{
		VfsNodeRelease(n2);
		err = NX_EBUSY;
		goto fail2;
	}

	if(VfsLookupDir(dst, &dn, &dname))
    {
        err = NX_ENOSRCH;
		goto fail2;
    }

	if(sn->mount != dn->mount)
	{
        err = NX_EFAULT;
		goto fail3;
	}

	NX_MutexLock(&n1->lock);
	NX_MutexLock(&sn->lock);

	if(dn != sn)
    {
		NX_MutexLock(&dn->lock);
    }

	err = sn->mount->fs->rename(sn, sname, n1, dn, dname);
	if(err)
    {
		goto fail4;
    }

	err = sn->mount->fs->sync(sn);
	if(err)
    {
		goto fail4;
    }

	if(dn != sn)
    {
		err = dn->mount->fs->sync(dn);
    }

fail4:
	if(dn != sn)
    {
		NX_MutexUnlock(&dn->lock);
    }
	NX_MutexUnlock(&sn->lock);
	NX_MutexUnlock(&n1->lock);
fail3:
	VfsNodeRelease(dn);
fail2:
	VfsNodeRelease(sn);
fail1:
	VfsNodeRelease(n1);

	return err;
}

NX_Error NX_VfsUnlink(const char * path)
{
	NX_VfsNode * n, * dn;
	char * name;
	NX_Error err;

	if(!path)
    {
		return NX_EINVAL;
    }

	if(VfsNodeAcquire(path, &n))
    {
		return NX_ENOSRCH;
    }

	if(n->type == NX_VFS_NODE_TYPE_DIR)
	{
		VfsNodeRelease(n);
		return NX_ENORES;
	}

	if((n->flags == NX_VFS_NODE_FLAG_ROOT) || (NX_AtomicGet(&n->refcnt) >= 2))
	{
		VfsNodeRelease(n);
		return NX_EBUSY;
	}

	if(VfsNodeAccess(n, NX_VFS_W_OK))
	{
		VfsNodeRelease(n);
		return NX_EPERM;
	}

	if(VfsLookupDir(path, &dn, &name))
	{
		VfsNodeRelease(n);
		return NX_ENOSRCH;
	}

	NX_MutexLock(&n->lock);
	err = n->mount->fs->truncate(n, 0);
	if(err)
    {
		goto fail1;
    }

	err = n->mount->fs->sync(n);
	if(err)
    {
		goto fail1;
    }

	NX_MutexLock(&dn->lock);
	err = dn->mount->fs->remove(dn, n, name);
	if(err)
    {
		goto fail2;
    }
	err = dn->mount->fs->sync(dn);

fail2:
	NX_MutexUnlock(&dn->lock);
fail1:
	NX_MutexUnlock(&n->lock);
	VfsNodeRelease(dn);
	VfsNodeRelease(n);

	return err;
}

NX_Error NX_VfsAccess(const char * path, NX_U32 mode)
{
	NX_VfsNode * n;
	NX_Error err = NX_EOK;

	if(!path)
    {
		return NX_EINVAL;
    }

	if(VfsNodeAcquire(path, &n))
    {
		return NX_ENOSRCH;
    }

	if (VfsNodeAccess(n, mode))
    {
        err = NX_EPERM;
    }
	
    VfsNodeRelease(n);

	return err;
}

NX_Error NX_VfsChmod(const char * path, NX_U32 mode)
{
	NX_VfsNode * n;
	NX_Error err;

	if(!path)
    {
		return NX_EINVAL;
    }

	if(VfsNodeAcquire(path, &n))
    {
		return NX_ENOSRCH;
    }

	mode &= (NX_VFS_S_IRWXU | NX_VFS_S_IRWXG | NX_VFS_S_IRWXO);

	NX_MutexLock(&n->lock);
	err = n->mount->fs->chmod(n, mode);
	if(err)
    {
		goto fail;
    }
	err = n->mount->fs->sync(n);

fail:
	NX_MutexUnlock(&n->lock);
	VfsNodeRelease(n);

	return err;
}

NX_Error NX_VfsStat(const char * path, NX_VfsStatInfo * st)
{
	NX_VfsNode * n;
	NX_Error err = NX_EOK;

	if(!path || !st)
    {
		return NX_EINVAL;
    }

	if(VfsNodeAcquire(path, &n))
    {
		return NX_ENOSRCH;
    }
	err = VfsNodeStat(n, st);
	VfsNodeRelease(n);

	return err;
}

NX_PRIVATE void NX_VfsInit(void)
{
	int i;

	NX_ListInit(&NX_FileSystemMountList);
	NX_MutexInit(&NX_FileSystemMountLock);

	for(i = 0; i < NX_VFS_NODE_HASH_SIZE; i++)
	{
		NX_ListInit(&NX_FileSystemNodeList[i]);
		NX_MutexInit(&NX_FileSystemNodeLock[i]);
	}

    NX_VfsFileTableInit(&DefaultFileTable);
}

NX_MODS_CALL(NX_VfsInit);
