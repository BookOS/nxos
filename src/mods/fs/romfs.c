/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: rom file system
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-31      JasonHu           Init
 */

#include <mods/fs/romfs.h>
#include <mods/thirdpart/cpio.h>
#include <mm/alloc.h>
#include <utils/memory.h>
#include <utils/string.h>
#include <utils/log.h>

NX_PRIVATE NX_RomfsSystem romfsSystem = {NX_NULL};

NX_IMPORT unsigned char __hex_file_rootfs[];

#define ROOT_PATH       "/"
#define ROOT_PATH_LEN   (sizeof(ROOT_PATH))

NX_PRIVATE void NX_RomfsList(void)
{
    struct cpio_info info;
    cpio_info(romfsSystem.archive, &info);
    char *path;
    int i;
    char* file_buf;
    char *filename;
    NX_USize file_sz;

    if ((path = (char *)NX_MemAlloc(info.max_path_sz + ROOT_PATH_LEN)) == NX_NULL)
    {
        NX_LOG_E("no memory (%dKB) enough", (info.max_path_sz + ROOT_PATH_LEN) / NX_KB);
        return;
    }

    for (i = 0; i < info.file_count; ++i)
    {
        file_buf = cpio_get_entry(__hex_file_rootfs, i, (const char**)&filename, &file_sz);
        NX_StrCopy(path, ROOT_PATH);
        NX_StrCopy(path + ROOT_PATH_LEN - 1, filename);
        
        if (file_sz == 0)
        {
            NX_LOG_I("file: %s dir", path);
        }
        else
        {
            NX_LOG_I("file: %s file", path);
            NX_LOG_I("file: %s", file_buf);
        }
    }
    NX_MemFree(path);
}

NX_PUBLIC NX_Error NX_RomfsMount(const char *path, const char *devname, int flags)
{
    if (path == NX_NULL || devname == NX_NULL)
    {
        return NX_EINVAL;
    }

    romfsSystem.archive = __hex_file_rootfs;
    NX_RomfsList();
    return NX_EOK;
}

NX_PUBLIC NX_Error NX_RomfsUnmount(const char *path, int flags)
{
    if (path == NX_NULL)
    {
        return NX_EINVAL;
    }
    
    romfsSystem.archive = NX_NULL;
    return NX_EOK;
}

NX_PRIVATE NX_RomfsFile *NX_RomfsCreateFile(void *buf, NX_USize size)
{
    NX_RomfsFile *file = NX_MemAlloc(sizeof(NX_RomfsFile));
    if (file == NX_NULL)
    {
        return NX_NULL;
    }

    file->buf = (char *)buf;
    file->size = size;
    file->off = 0;
    return file;
}

NX_PUBLIC NX_Error NX_RomfsOpen(const char *path, int flags, NX_RomfsFile **outFile)
{
    NX_RomfsFile *file;
    unsigned long size;
    void *fileBuf;

    if (romfsSystem.archive == NX_NULL)
    {
        return NX_EFAULT;
    }

    if (path == NX_NULL)
    {
        return NX_EINVAL;
    }

    /* get file from cpio */
    fileBuf = cpio_get_file(romfsSystem.archive, path, &size);
    if (fileBuf == NX_NULL)
    {
        return NX_ENOSRCH;
    }
    
    if (size == 0)  /* is a dir */
    {
        return NX_EPERM;
    }

    file = NX_RomfsCreateFile(fileBuf, size);
    if (file == NX_NULL)
    {
        return NX_ENOMEM;
    }

    *outFile = file;
    
    return NX_EOK;
}

NX_PUBLIC NX_Error NX_RomfsClose(NX_RomfsFile *file)
{
    if (file == NX_NULL)
    {
        return NX_EINVAL;
    }
    NX_MemFree(file);
    return NX_EOK;
}

NX_PUBLIC NX_Error NX_RomfsRead(NX_RomfsFile *file, void *buf, NX_USize bytes, NX_USize *outBytes)
{
    NX_ISize chunk;

    if (romfsSystem.archive == NX_NULL)
    {
        return NX_EFAULT;
    }

    if (file == NX_NULL || buf == NX_NULL || !bytes)
    {
        return NX_EINVAL;
    }

    chunk = NX_MIN(bytes, file->size - file->off);
    if (chunk <= 0)
    {
        return NX_EFAULT;
    }

    NX_MemCopy(buf, file->buf + file->off, chunk);

    file->off += chunk;
    
    if (outBytes)
    {
        *outBytes = chunk;
    }

    return NX_EOK;
}

NX_PUBLIC NX_Error NX_RomfsSeek(NX_RomfsFile *file, NX_Offset off, int flags, NX_Offset *outOff)
{
    NX_Offset offVal = 0;

    if (romfsSystem.archive == NX_NULL)
    {
        return NX_EFAULT;
    }

    if (file == NX_NULL)
    {
        return NX_EINVAL;
    }

    switch (flags)
    {
    case NX_ROMFS_SEEK_SET:
        offVal = off;
        break;

    case NX_ROMFS_SEEK_CUR:
        offVal = file->off + off;
        break;

    case NX_ROMFS_SEEK_END:
        offVal = file->size + off;
        break;

    default:
        break;
    }

    if (offVal < 0)
    {
        return NX_EFAULT;
    }

    file->off = offVal;

    if (outOff)
    {
        *outOff = file->off;
    }

    return NX_EOK;
}
