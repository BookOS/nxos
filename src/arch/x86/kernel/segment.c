/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: x86 segement
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-9-17      JasonHu           Init
 */

#include <segment.h>
#include <tss.h>

void CPU_LoadGDT(NX_UArch NX_Size, NX_UArch gdtr);

struct CPU_Segment
{
    NX_U16 limitLow, baseLow;
    NX_U8 baseMid, accessRight;
    NX_U8 limitHigh, baseHigh;
};

NX_PRIVATE void SetSegment(struct CPU_Segment *seg, NX_UArch limit,
                        NX_UArch base, NX_UArch attributes)
{
    seg->limitLow    = limit & 0xffff;
    seg->baseLow     = base & 0xffff;
    seg->baseMid     = (base >> 16) & 0xff;
    seg->accessRight = attributes & 0xff;
    seg->limitHigh   = ((limit >> 16) & 0x0f) | ((attributes >> 8) & 0xf0);
    seg->baseHigh    = (base >> 24) & 0xff;
}

void CPU_InitSegment(void)
{
    /* Global segment table */
    struct CPU_Segment *gdt = (struct CPU_Segment *) GDT_VADDR;

    int i;
    for (i = 0; i <= GDT_LIMIT / 8; i++)
    {
        SetSegment(GDT_OFF2PTR(gdt, i), 0, 0, 0);
    }

    SetSegment(GDT_OFF2PTR(gdt, INDEX_KERNEL_CODE), GDT_BOUND_TOP, GDT_BOUND_BOTTOM, GDT_KERNEL_CODE_ATTR);
    SetSegment(GDT_OFF2PTR(gdt, INDEX_KERNEL_DATA), GDT_BOUND_TOP, GDT_BOUND_BOTTOM, GDT_KERNEL_DATA_ATTR);

    struct CPU_TSS *tss = CPU_GetTSS();
    SetSegment(GDT_OFF2PTR(gdt, INDEX_TSS), sizeof(struct CPU_TSS) - 1, (NX_UArch)tss, GDT_TSS_ATTR);

    SetSegment(GDT_OFF2PTR(gdt, INDEX_USER_CODE), GDT_BOUND_TOP, GDT_BOUND_BOTTOM, GDT_USER_CODE_ATTR);
    SetSegment(GDT_OFF2PTR(gdt, INDEX_USER_DATA), GDT_BOUND_TOP, GDT_BOUND_BOTTOM, GDT_USER_DATA_ATTR);

    SetSegment(GDT_OFF2PTR(gdt, INDEX_USER_TLS), GDT_BOUND_TOP, GDT_BOUND_BOTTOM, GDT_USER_TLS_ATTR);

    CPU_LoadGDT(GDT_LIMIT, GDT_VADDR);
}
