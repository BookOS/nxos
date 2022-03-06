/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: PC32 boot setting 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-9-17      JasonHu           Init
 */

#include <multiboot2.h>
#include <boot.h>
#include <utils/string.h>

NX_INLINE void BootModInit(struct multiboot_tag *tag);
NX_INLINE void BootMemModInit(struct multiboot_tag *tag);

int NX_HalBootSetting(unsigned long magic, unsigned long addr)
{
    // whether a multiboot
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC || addr & 7)
    {
        return -1;
    }
    struct multiboot_tag *tag;
    
    BootModInfoInit();

    for (tag = (struct multiboot_tag*)(addr + 8);
        tag->type != MULTIBOOT_TAG_TYPE_END;
        tag = (struct multiboot_tag*)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7)))
    {
        switch (tag->type) {
        case MULTIBOOT_TAG_TYPE_MODULE:
            BootModInit(tag);
            break;
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            BootMemModInit(tag);
            break;
        }
    }
    return 0;
}

#define CmdlineIs(cmd) (!NX_StrCmp(((struct multiboot_tag_module *)tag)->cmdline, cmd))

NX_INLINE void BootModInit(struct multiboot_tag *tag) {
    struct BootModInfo *modInfo = (struct BootModInfo *)BOOT_INFO_ADDR;
    int index = modInfo->modNum;

    if (index >= MAX_BOOT_MODS_NUM
        || modInfo->modSize + ((struct multiboot_tag_module *)tag)->size > MAX_BOOT_MODS_SIZE)
    {
        return;
    }

    modInfo->modules[index].size = ((struct multiboot_tag_module *)tag)->size;
    modInfo->modules[index].start = ((struct multiboot_tag_module *)tag)->mod_start;
    modInfo->modules[index].end = ((struct multiboot_tag_module *)tag)->mod_end;

    if (CmdlineIs("initrd"))
    {
        modInfo->modules[index].type = BOOT_MOD_INITRD;
    }
    else
    {
        modInfo->modules[index].type = BOOT_MOD_UNKNOWN;
    }

    modInfo->modSize += modInfo->modules[index].size;
    ++modInfo->modNum;
}

#undef CmdlineIs

NX_INLINE void BootMemModInit(struct multiboot_tag *tag)
{
    unsigned long memUpper = ((struct multiboot_tag_basic_meminfo *)tag)->mem_upper;
    unsigned long memLower = ((struct multiboot_tag_basic_meminfo *)tag)->mem_lower;
    *((unsigned int *)0x000001000) = ((memUpper - memLower) << 10) + 0x100000;
}
