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

#include <mm/vmspace.h>
#include <mm/page.h>
#include <mm/alloc.h>
#include <mm/mmu.h>
#include <process/process.h>
#include <xbook/debug.h>

#define NX_LOG_NAME "vmspace"
#include <utils/log.h>

/* remove node with destroy node */
#define VMNODE_REMOVE_WITH_DESTORY 0x01

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
    NX_Addr stackEnd)
{
    if (!space)
    {
        return NX_EINVAL;
    }
    NX_ListInit(&space->spaceNodeList);
    NX_MmuInit(&space->mmu, NX_NULL, 0, 0, 0);
    NX_SpinInit(&space->spinLock);
    
    space->spaceBase = spaceBase;
    space->spaceTop = spaceTop;
    space->imageStart = imageStart;
    space->imageEnd = imageEnd;
    space->heapStart = heapStart;
    space->heapEnd = heapEnd;
    space->mapStart = mapStart;
    space->mapEnd = mapEnd;
    space->stackStart = stackStart;
    space->stackEnd = stackEnd;
    return NX_EOK;
}

NX_Error NX_VmspaceListNodes(NX_Vmspace *space)
{
    NX_Vmnode *node;
    NX_UArch level;

    if (!space)
    {
        return NX_EINVAL;
    }

    NX_SpinLockIRQ(&space->spinLock, &level);
    NX_LOG_I("==> Vmspace at %p list nodes count %d .", space, NX_ListLength(&space->spaceNodeList));
    NX_ListForEachEntry(node, &space->spaceNodeList, list)
    {
        NX_LOG_I("    range: [%p-%p), attr: %x, flags: %x", 
            node->start, node->end, node->attr, node->flags);
    }
    NX_SpinUnlockIRQ(&space->spinLock, level);
    NX_LOG_I("==> Vmspace at %p list nodes done .", space);
    return NX_EOK;
}

NX_PRIVATE NX_Vmnode *VmnodeCreate(
    NX_Addr addr,
    NX_Size size,
    NX_U32 attr,
    NX_U32 flags)
{
    NX_Vmnode *node = NX_MemAlloc(sizeof(NX_Vmnode));
    if (node == NX_NULL)
    {
        return NX_NULL;
    }

    node->start = addr;
    node->end = addr + size;
    node->attr = attr;
    node->flags = flags;
    node->space = NX_NULL;
    NX_ListInit(&node->list);
    return node;
}

NX_PRIVATE NX_Error VmnodeDestroy(NX_Vmnode *node)
{
    if (!node)
    {
        return NX_EINVAL;
    }
    NX_MemFree(node);
    return NX_EOK;
}

NX_PRIVATE void VmspaceInsertLinearOrder(NX_Vmspace *space, NX_Vmnode *node)
{
    if (NX_ListEmpty(&space->spaceNodeList))
    {
        NX_ListAdd(&node->list, &space->spaceNodeList);
    }
    else
    {
        /* find prev and insert after prev */
        NX_Vmnode *prevNode = NX_NULL;
        NX_Vmnode *tmpNode;

        NX_ListForEachEntry(tmpNode, &space->spaceNodeList, list)
        {
            if (tmpNode->end <= node->start)
            {
                if (prevNode == NX_NULL) /* first prev node */
                {
                    prevNode = tmpNode;
                }
                else
                {
                    if (tmpNode->end > prevNode->end) /* find prev node end near node */
                    {
                        prevNode = tmpNode;
                    }
                }
            }
        }

        if (prevNode == NX_NULL) /* no prev, insert at head */
        {
            NX_ListAdd(&node->list, &space->spaceNodeList);
        }
        else    /* insert node after prev */
        {
            NX_ListAddAfter(&node->list, &prevNode->list);
        }
    }
}

NX_PRIVATE NX_Error VmspaceInsertNodeLocked(NX_Vmspace *space, NX_Vmnode *node)
{
    if (!space || !node)
    {
        return NX_EINVAL;
    }

    VmspaceInsertLinearOrder(space, node);
    node->space = space;
    return NX_EOK;
}

NX_PRIVATE NX_Error VmspaceInsertNode(NX_Vmspace *space, NX_Vmnode *node)
{
    NX_UArch level;
    NX_Error err;
    NX_SpinLockIRQ(&space->spinLock, &level);
    err = VmspaceInsertNodeLocked(space, node);
    NX_SpinUnlockIRQ(&space->spinLock, level);
    return err;
}

NX_PRIVATE NX_Error VmspaceRemoveNodeLocked(NX_Vmspace *space, NX_Vmnode *node, NX_U32 flags)
{
    if (!space || !node)
    {
        return NX_EINVAL;
    }

    if (node->space != space)
    {
        return NX_EFAULT;
    }

    NX_ListDel(&node->list);
    node->space = NX_NULL;

    if (flags & VMNODE_REMOVE_WITH_DESTORY)
    {
        NX_ASSERT(VmnodeDestroy(node) == NX_EOK);
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error VmspaceRemoveNode(NX_Vmspace *space, NX_Vmnode *node, NX_U32 flags)
{
    NX_UArch level;
    NX_Error err;
    NX_SpinLockIRQ(&space->spinLock, &level);
    err = VmspaceRemoveNodeLocked(space, node, flags);
    NX_SpinUnlockIRQ(&space->spinLock, level);
    return err;
}

/**
 * merge condition:
 * 1. near addr
 * 2. same attr and flags
 */
NX_PRIVATE NX_Error VmspaceMergeNode(NX_Vmspace *space, NX_Vmnode *node)
{
    NX_UArch level;
    NX_Vmnode *prevNode, *nextNode;

    if (!space || !node)
    {
        return NX_EINVAL;
    }

    NX_SpinLockIRQ(&space->spinLock, &level);
    /* merge with prev node */
    NX_ListForEachEntry(prevNode, &space->spaceNodeList, list)
    {
        if (prevNode->end == node->start) /* near prev node */
        {
            if (prevNode->attr == node->attr && prevNode->flags == node->flags) /* same node */
            {
                node->start = prevNode->start;
                NX_ASSERT(VmspaceRemoveNodeLocked(space, prevNode, VMNODE_REMOVE_WITH_DESTORY) == NX_EOK);
                break;
            }
        }
    }

    /* merge with next node */
    NX_ListForEachEntry(nextNode, &space->spaceNodeList, list)
    {
        if (nextNode->start == node->end) /* near next node */
        {
            if (nextNode->attr == node->attr && nextNode->flags == node->flags) /* same node */
            {
                node->end = nextNode->end;
                NX_ASSERT(VmspaceRemoveNodeLocked(space, nextNode, VMNODE_REMOVE_WITH_DESTORY) == NX_EOK);
                break;
            }
        }
    }
    
    NX_SpinUnlockIRQ(&space->spinLock, level);
    return NX_EOK;
}

NX_PRIVATE NX_Error VmspaceSplitNode(NX_Vmspace *space, NX_Vmnode *node, NX_Addr addr, NX_Size size)
{
    NX_UArch level;
    NX_Addr start = addr;
    NX_Addr end = addr + size;
    NX_Addr oldStart;
    NX_Addr oldEnd;

    if (!space || !node)
    {
        return NX_EINVAL;
    }

    oldStart = node->start;
    oldEnd = node->end;

    /* no need to split node */
    if (node->start == start && node->end == end)
    {
        return NX_EOK;
    }
    else
    {
        /* spilt as prev, node, next. */
        NX_Addr prevStart = node->start;
        NX_Addr prevEnd = start;
        NX_Addr nextStart = end;
        NX_Addr nextEnd = node->end;
        NX_Vmnode *prevNode = NX_NULL, *nextNode = NX_NULL;

        NX_SpinLockIRQ(&space->spinLock, &level);
        /* create prev node */
        if (prevStart != prevEnd)
        {
            prevNode = VmnodeCreate(prevStart, prevEnd - prevStart, node->attr, node->flags);
            if (prevNode == NX_NULL)
            {
                NX_SpinUnlockIRQ(&space->spinLock, level);
                return NX_ENOMEM;
            }
        }

        /* create next node */
        if (nextStart != nextEnd)
        {
            nextNode = VmnodeCreate(nextStart, nextEnd - nextStart, node->attr, node->flags);
            if (nextNode == NX_NULL)
            {
                if (prevNode != NX_NULL)
                {
                    NX_ASSERT(VmnodeDestroy(prevNode) == NX_EOK);
                }
                NX_SpinUnlockIRQ(&space->spinLock, level);
                return NX_ENOMEM;
            }
        }

        /* insert prev node */
        if (prevNode != NX_NULL)
        {
            node->start = prevEnd;
            if (VmspaceInsertNodeLocked(space, prevNode) != NX_EOK)
            {
                node->start = oldStart;
                NX_ASSERT(VmnodeDestroy(prevNode) == NX_EOK);
                if (nextNode != NX_NULL)
                {
                    NX_ASSERT(VmnodeDestroy(nextNode) == NX_EOK);
                }
                NX_SpinUnlockIRQ(&space->spinLock, level);
                return NX_EFAULT;
            }
        }

        /* insert next node */
        if (nextNode != NX_NULL)
        {
            node->end = nextStart;
            if (VmspaceInsertNodeLocked(space, nextNode) != NX_EOK)
            {
                node->start = oldStart;
                node->end = oldEnd;

                NX_ASSERT(VmnodeDestroy(nextNode) == NX_EOK);
                if (prevNode != NX_NULL)
                {
                    NX_ASSERT(VmspaceRemoveNodeLocked(space, prevNode, VMNODE_REMOVE_WITH_DESTORY));
                }
                NX_SpinUnlockIRQ(&space->spinLock, level);
                return NX_EFAULT;
            }
        }
        NX_SpinUnlockIRQ(&space->spinLock, level);
    }
    return NX_EOK;
}

NX_PRIVATE NX_Vmnode *VmspaceFindNode(NX_Vmspace *space, NX_Addr addr, NX_Size size)
{
    NX_Vmnode *node;
    NX_UArch level;

    NX_SpinLockIRQ(&space->spinLock, &level);
    NX_ListForEachEntry(node, &space->spaceNodeList, list)
    {
        if (addr >= node->start && addr + size <= node->end)
        {
            NX_SpinUnlockIRQ(&space->spinLock, level);
            return node;
        }
    }
    NX_SpinUnlockIRQ(&space->spinLock, level);
    return NX_NULL;
}

NX_PRIVATE NX_Bool VmspaceCheckAddrCollisions(NX_Vmspace *space, NX_Addr addr, NX_Size size)
{
    NX_ASSERT(space);
    NX_Addr start = addr;
    NX_Addr end = start + size;
    NX_Vmnode *node;
    NX_UArch level;

    NX_SpinLockIRQ(&space->spinLock, &level);
    NX_ListForEachEntry(node, &space->spaceNodeList, list)
    {
        /* start in range [node_start, node_end) or end in range (node_start, node_end] */
        if ((start >= node->start && start < node->end) ||
            (end > node->start && end <= node->end))
        {
            NX_SpinUnlockIRQ(&space->spinLock, level);
            return NX_True;
        }
    }
    NX_SpinUnlockIRQ(&space->spinLock, level);
    return NX_False;
}

NX_PRIVATE NX_Error GetAddrFromMapArea(NX_Vmspace *space, NX_Size size, NX_Addr *outAddr)
{
    NX_Vmnode *node;
    NX_Addr freeAddr;
    NX_UArch level;

    NX_ASSERT(space);
    NX_ASSERT(size);

    NX_SpinLockIRQ(&space->spinLock, &level);
    /* find the addr */
    freeAddr = space->mapStart; /* begin from start */
    NX_ListForEachEntry(node, &space->spaceNodeList, list)
    {
        if (node->start >= space->mapStart && node->end <= space->mapEnd) /* in map area */
        {
            if (freeAddr + size > space->mapEnd) /* no enough memory */
            {    
                NX_SpinUnlockIRQ(&space->spinLock, level);
                return NX_ENOMEM;
            }
            if (freeAddr + size <= node->start) /* get add before a node */
            {
                *outAddr = freeAddr;    
                NX_SpinUnlockIRQ(&space->spinLock, level);
                return NX_EOK;
            }
            /* set free addr as next node end */
            freeAddr = node->end;
        }
    }

    *outAddr = freeAddr;
    NX_SpinUnlockIRQ(&space->spinLock, level);
    return NX_EOK;
}

/**
 * @param vaddr if vaddr == 0: alloc a virtual addr and map.
 * @param paddr if paddr == 0: map without paddr.
 */
NX_Error __VmspaceMap(NX_Vmspace *space,
    NX_Addr vaddr,
    NX_Addr paddr,
    NX_Size size,
    NX_U32 attr,
    NX_U32 flags,
    void **outAddr)
{
    NX_Vmnode *node;
    void *mapAddr = NX_NULL;

    if (!space || !size || !attr)
    {
        return NX_EINVAL;
    }

    /* addr not in space range */
    if (vaddr && (vaddr < space->spaceBase || vaddr > space->spaceTop - size))
    {
        NX_LOG_E("map: vaddr %p and size %p error !", vaddr);
        return NX_EINVAL;
    }
    
    if (!vaddr)
    {
        if (GetAddrFromMapArea(space, size, &vaddr) != NX_EOK)
        {
            return NX_ENOMEM;
        }
    }

    /* addr and size page align */
    vaddr = NX_PAGE_ALIGNDOWN(vaddr);
    size = NX_PAGE_ALIGNUP(size);
    
    if (VmspaceCheckAddrCollisions(space, vaddr, size) == NX_True)
    {
        NX_LOG_E("map addr:%p size:%p collide !", vaddr, size);
        return NX_EINVAL;
    }

    /* add node */
    node = VmnodeCreate(vaddr, size, attr, flags);
    if (node == NX_NULL)
    {
        return NX_ENOMEM;
    }

    if (VmspaceInsertNode(space, node) != NX_EOK)
    {
        NX_ASSERT(VmnodeDestroy(node) == NX_EOK);
        return NX_EFAULT;
    }

    /* if no delay, map addr */
    if (flags & NX_VMSPACE_DELAY_MAP)
    {
        mapAddr = (void *)vaddr;        
    }
    else
    {
        if (paddr)
        {
            /* increase page reference, if not in system, igonre it. */
            NX_PageIncrease(paddr & NX_PAGE_ADDR_MASK);
            mapAddr = NX_MmuMapPageWithPhy(&space->mmu, vaddr, paddr, size, attr);
        }
        else
        {
            mapAddr = NX_MmuMapPage(&space->mmu, vaddr, size, attr);
        }

        if (mapAddr == NX_NULL)
        {
            NX_ASSERT(VmspaceRemoveNode(space, node, VMNODE_REMOVE_WITH_DESTORY) == NX_EOK);
            return NX_ENOMEM;
        }
    }

    /* merge node */
    NX_ASSERT(VmspaceMergeNode(space, node) == NX_EOK);

    if (outAddr)
    {
        *outAddr = mapAddr;
    }
    return NX_EOK;
}

NX_Error NX_VmspaceMap(NX_Vmspace *space,
    NX_Addr addr,
    NX_Size size,
    NX_U32 attr,
    NX_U32 flags,
    void **outAddr)
{
    return __VmspaceMap(space, addr, 0, size, attr, flags, outAddr);
}

NX_Error NX_VmspaceMapWithPhy(NX_Vmspace *space,
    NX_Addr vaddr,
    NX_Addr paddr,
    NX_Size size,
    NX_U32 attr,
    NX_U32 flags,
    void **outAddr)
{
    return __VmspaceMap(space, vaddr, paddr, size, attr, flags, outAddr);
}

NX_Error NX_VmspaceUnmap(NX_Vmspace *space, NX_Addr addr, NX_Size size)
{
    NX_Vmnode *node;
    NX_Error err;

    if (!space || !addr || !size)
    {
        return NX_EINVAL;
    }
    
    /* addr must page aligned and in space */
    if ((addr & NX_PAGE_MASK) || (addr < space->spaceBase) || (addr >= space->spaceTop) || (addr + size) > space->spaceTop)
    {
        NX_LOG_E("unmap: addr %p and size %p error on addr !", addr, size);
        return NX_EINVAL;
    }
    
    size = NX_PAGE_ALIGNUP(size);
    
    /* check addr mapped */
    node = VmspaceFindNode(space, addr, size);
    if (node == NX_NULL)
    {
        NX_LOG_E("unmap: addr %p size %p not found !", addr, size);
        return NX_EFAULT;
    }

    /* split node */
    err = VmspaceSplitNode(space, node, addr , size);
    if (err != NX_EOK)
    {
        NX_LOG_E("unmap: addr %p size %p node %p split error with %d !", addr, size, node, err);
        return NX_ENOMEM;
    }

    /* remove node from space */
    err = VmspaceRemoveNode(space, node, !VMNODE_REMOVE_WITH_DESTORY);
    if (err != NX_EOK)
    {
        NX_LOG_E("unmap: addr %p size %p node %p remove error with %d !", addr, size, node, err);
        NX_ASSERT(VmspaceMergeNode(space, node) == NX_EOK);
        return NX_EFAULT;
    }
    
    /* unmap addr */
    err = NX_MmuUnmapPage(&space->mmu, addr, size);
    if (err != NX_EOK)
    {
        NX_LOG_E("unmap: addr %p size %p unmap error with %d !", addr, size, err);
        NX_ASSERT(VmspaceInsertNode(space, node) == NX_EOK);
        NX_ASSERT(VmspaceMergeNode(space, node) == NX_EOK);
        return NX_EFAULT;
    }

    /* destroy node at last */
    NX_ASSERT(VmnodeDestroy(node) == NX_EOK);
    return NX_EOK;
}

NX_Error NX_VmspaceExit(NX_Vmspace *space)
{
    NX_UArch level;
    NX_Vmnode *node, *next;

    if (!space)
    {
        return NX_EINVAL;
    }
    
    NX_SpinLockIRQ(&space->spinLock, &level);
    NX_ListForEachEntrySafe(node, next, &space->spaceNodeList, list)
    {
        /* unmap addr */
        NX_ASSERT(NX_MmuUnmapPage(&space->mmu, node->start, node->end - node->start) == NX_EOK);
        /* remove node */
        NX_ASSERT(VmspaceRemoveNodeLocked(space, node, VMNODE_REMOVE_WITH_DESTORY) == NX_EOK);
    }
    NX_SpinUnlockIRQ(&space->spinLock, level);

    /* free page table */
    NX_ASSERT(NX_ProcessFreePageTable(space) == NX_EOK);
    return NX_EOK;
}

NX_Addr NX_VmspaceVirToPhy(NX_Vmspace *space, NX_Addr virAddr)
{
    if (!space)
    {
        return 0;
    }
    if (!space->mmu.table)
    {
        return 0;
    }
    
    if (virAddr < space->spaceBase || virAddr >= space->spaceTop)
    {
        return 0;
    }

    return (NX_Addr)NX_MmuVir2Phy(&space->mmu, virAddr);
}
