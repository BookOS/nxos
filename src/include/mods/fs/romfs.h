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

#ifndef __MODS_FS_ROMFS__
#define __MODS_FS_ROMFS__

#include <xbook.h>

#define NX_ROMFS_SEEK_SET   0
#define NX_ROMFS_SEEK_CUR   1
#define NX_ROMFS_SEEK_END   2

struct NX_RomfsFile
{
    char *buf;
    NX_USize size;
    NX_Offset off;
};
typedef struct NX_RomfsFile NX_RomfsFile;

struct NX_RomfsDir
{
};
typedef struct NX_RomfsDir NX_RomfsDir;

struct NX_RomfsSystem
{
    void *archive;  /* cpio archive */
};
typedef struct NX_RomfsSystem NX_RomfsSystem;

NX_PUBLIC NX_Error NX_RomfsMount(const char *path, const char *devname, int flags);
NX_PUBLIC NX_Error NX_RomfsUnmount(const char *path, int flags);

NX_PUBLIC NX_Error NX_RomfsOpen(const char *path, int flags, NX_RomfsFile **outFile);
NX_PUBLIC NX_Error NX_RomfsClose(NX_RomfsFile *file);

NX_PUBLIC NX_Error NX_RomfsRead(NX_RomfsFile *file, void *buf, NX_USize bytes, NX_USize *outBytes);
NX_PUBLIC NX_Error NX_RomfsSeek(NX_RomfsFile *file, NX_Offset off, int flags, NX_Offset *outOff);

#endif /* __MODS_FS_ROMFS__ */

