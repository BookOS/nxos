/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: virtual memory space for user
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-2-14      JasonHu           Init
 */

#ifndef __MM_VMSPACE__
#define __MM_VMSPACE__

#include <xbook.h>
#include <mm/mmu.h>
#include <utils/list.h>
#include <sched/spin.h>

/* vmspace flags */
#define NX_VMSPACE_DELAY_MAP 0x01   /* delay map phy addr when read/write */

struct NX_Vmspace
{
    NX_Mmu mmu;
    NX_List spaceNodeList;
    NX_Spin spinLock;

    NX_Addr spaceBase;   /* user space area */
    NX_Addr spaceTop;
    NX_Addr imageStart;   /* image: code & data */
    NX_Addr imageEnd;
    NX_Addr heapStart;
    NX_Addr heapEnd;
    NX_Addr mapStart;
    NX_Addr mapEnd;
    NX_Addr stackStart;
    NX_Addr stackEnd;
};
typedef struct NX_Vmspace NX_Vmspace;

struct NX_Vmnode
{
    NX_List list;   /* vmnode list */
    NX_Addr start;  /* node area: [start, end) */
    NX_Addr end;
    NX_UArch attr;
    NX_U32 flags;
    NX_Vmspace *space;
};
typedef struct NX_Vmnode NX_Vmnode;

NX_Error NX_VmspaceInit(NX_Vmspace *space,
    NX_Addr spaceBase,
    NX_Addr spaceTop,
    NX_Addr imageStart,
    NX_Addr imageEnd,
    NX_Addr heapStart,
    NX_Addr heapEnd,
    NX_Addr mapStart,
    NX_Addr mapEnd,
    NX_Addr stackStart,
    NX_Addr stackEnd);
NX_Error NX_VmspaceExit(NX_Vmspace *space);

NX_Error NX_VmspaceMap(NX_Vmspace *space,
    NX_Addr addr,
    NX_Size size,
    NX_UArch attr,
    NX_U32 flags,
    void **outAddr);

NX_Error NX_VmspaceMapWithPhy(NX_Vmspace *space,
    NX_Addr vaddr,
    NX_Addr paddr,
    NX_Size size,
    NX_UArch attr,
    NX_U32 flags,
    void **outAddr);

NX_Error NX_VmspaceUnmap(NX_Vmspace *space, NX_Addr addr, NX_Size size);

NX_Error NX_VmspaceListNodes(NX_Vmspace *space);

NX_Addr NX_VmspaceVirToPhy(NX_Vmspace *space, NX_Addr virAddr);

#endif /* __MM_VMSPACE__ */
