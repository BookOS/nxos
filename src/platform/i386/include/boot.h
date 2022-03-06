/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Boot info
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-19     JasonHu           Init
 */

#ifndef __PLATFROM_BOOT__
#define __PLATFROM_BOOT__

#include <xbook.h>

#define BOOT_INFO_ADDR 0x3F1000

#define MAX_BOOT_MODS_NUM 1
#define MAX_BOOT_MODS_SIZE (1 * NX_MB)

enum BootModType
{
	// Unknown type
	BOOT_MOD_UNKNOWN = 0,
	// Initrd image type
	BOOT_MOD_INITRD = 1,
};

struct BootModInfo
{
	unsigned int modNum;
	unsigned int modSize;
	struct
    {
		unsigned int type;
		unsigned int size;
		unsigned int start;
		unsigned int end;
	} modules[MAX_BOOT_MODS_NUM];
} __attribute__ ((packed));

NX_INLINE void BootModInfoInit()
{
	struct BootModInfo *modInfo = (struct BootModInfo *)BOOT_INFO_ADDR;
	modInfo->modNum = 0;
	modInfo->modSize = 0;
}

NX_INLINE void *BootModInfoFind(unsigned long addr, enum BootModType type)
{
	int i;
	struct BootModInfo *modInfo;
	modInfo = (struct BootModInfo *)(addr + BOOT_INFO_ADDR);

	for (i = 0; i < modInfo->modNum; ++i)
    {
		if (modInfo->modules[i].type == type)
        {
			return (void*)(addr + modInfo->modules[i].start);
		}
	}

	return (void*)0;
}

#endif /* __PLATFROM_BOOT__ */
