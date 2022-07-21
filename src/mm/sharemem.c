/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: share memory
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-07-09     JasonHu           Init
 */

#include <base/sharemem.h>
#include <base/spin.h>
#include <base/malloc.h>
#include <base/page.h>
#include <base/thread.h>
#include <base/vmspace.h>
#include <base/string.h>
#define NX_LOG_NAME "shm"
#include <base/log.h>
#include <base/debug.h>

NX_PRIVATE NX_LIST_HEAD(shareMemListHead);
NX_PRIVATE NX_SPIN_DEFINE_UNLOCKED(shareMemLock);

NX_PRIVATE NX_ShareMem * NX_ShareMemSearchByName(const char * name)
{
    NX_ShareMem * shm;
    NX_UArch level;
    NX_SpinLockIRQ(&shareMemLock, &level);
    NX_ListForEachEntry (shm, &shareMemListHead, list)
    {
        if (NX_StrCmp(shm->name, name) == 0)
        {
            NX_SpinUnlockIRQ(&shareMemLock, level);
            return shm;
        }
    }
    NX_SpinUnlockIRQ(&shareMemLock, level);
    return NX_NULL;
}

NX_PRIVATE NX_ShareMem * NX_ShareMemSearchByAddr(NX_Addr paddr)
{
    NX_ShareMem * shm;
    NX_UArch level;
    NX_SpinLockIRQ(&shareMemLock, &level);
    NX_ListForEachEntry (shm, &shareMemListHead, list)
    {
        if (shm->pageAddr == paddr)
        {
            NX_SpinUnlockIRQ(&shareMemLock, level);
            return shm;
        }
    }
    NX_SpinUnlockIRQ(&shareMemLock, level);
    return NX_NULL;
}

NX_ShareMem * NX_ShareMemOpen(const char * name, NX_Size size, NX_U32 flags)
{
    NX_Size pageCount;
    NX_ShareMem * shm;

    if (!name || !size || size > NX_SHAREMEM_MAX_SIZE)
    {
        return NX_NULL;
    }

    if ((shm = NX_ShareMemSearchByName(name)) != NX_NULL) /* share memory exist */
    {
        if (flags & NX_SHAREMEM_CREATE_NEW) /* share memory must not exist */
        {
            return NX_NULL;
        }
        if (size > shm->size)
        {
            return NX_NULL;
        }
        NX_AtomicInc(&shm->reference);
        return shm; /* return share mem */
    }

    pageCount = NX_DIV_ROUND_UP(size, NX_PAGE_SIZE);

    shm = NX_MemAlloc(sizeof(NX_ShareMem));
    if (!shm)
    {
        return NX_NULL;
    }
    
    shm->pageAddr = (NX_Addr)NX_PageAlloc(pageCount);
    if (!shm->pageAddr)
    {
        NX_MemFree(shm);
        return NX_NULL;
    }

    NX_StrCopyN(shm->name, name, NX_SHAREMEM_NAME_LEN);
    shm->size = pageCount * NX_PAGE_SIZE;
    NX_AtomicSet(&shm->reference, 1);

    NX_UArch level;
    NX_SpinLockIRQ(&shareMemLock, &level);
    NX_ListAdd(&shm->list, &shareMemListHead);
    NX_SpinUnlockIRQ(&shareMemLock, level);

    return shm;
}

NX_Error NX_ShareMemClose(NX_ShareMem * shm)
{
    if (!shm)
    {
        return NX_EINVAL;
    }

    if (!shm->pageAddr)
    {
        return NX_EPERM;
    }
    
    NX_AtomicDec(&shm->reference);

    if (NX_AtomicGet(&shm->reference) > 0) /* this close success */
    {
        return NX_EOK;
    }

    NX_UArch level;
    NX_SpinLockIRQ(&shareMemLock, &level);
    NX_ListDel(&shm->list);
    NX_SpinUnlockIRQ(&shareMemLock, level);

    NX_PageFree((void *)shm->pageAddr);
    shm->pageAddr = 0;
    NX_MemFree(shm);
    return NX_EOK;
}

NX_Error NX_ShareMemMap(NX_ShareMem * shm, void ** outMapAddr)
{
    NX_Thread * self;
    NX_Process * process;
    void * mapAddr = NX_NULL;
    NX_Error err;

    if (!shm)
    {
        return NX_EINVAL;
    }

    if (!shm->pageAddr)
    {
        return NX_EPERM;
    }

    self = NX_ThreadSelf();
    process = NX_ThreadGetProcess(self);
    
    if (process == NX_NULL)
    {
        return NX_EPERM;
    }

    if ((err = NX_VmspaceMapWithPhy(&process->vmspace, 0, shm->pageAddr, shm->size, NX_PAGE_ATTR_USER, NX_VMSPACE_SHAREMEM, &mapAddr)) != NX_EOK)
    {
        return err;
    }

    NX_AtomicInc(&shm->reference);

    if (outMapAddr)
    {
        *outMapAddr = mapAddr;
    }

    return NX_EOK;
}

NX_Error NX_ShareMemUnmap(void * addr)
{
    NX_Addr paddr;
    NX_Addr vaddr;
    NX_Thread * self;
    NX_Process * process;
    NX_Error err = NX_EOK;

    if (!addr)
    {
        return NX_EINVAL;
    }

    self = NX_ThreadSelf();
    process = NX_ThreadGetProcess(self);

    if (process == NX_NULL)
    {
        return NX_EPERM;
    }

    vaddr = ((NX_Addr)addr) & NX_PAGE_ADDR_MASK;
    paddr = NX_VmspaceVirToPhy(&process->vmspace, vaddr);

    NX_ShareMem * shm = NX_ShareMemSearchByAddr(paddr);
    if (!shm)
    {
        NX_LOG_E("share memory addr %p not found!", vaddr);
        return NX_ENOSRCH;
    }

    /* unmap space */
    if (NX_VmspaceUnmap(&process->vmspace, vaddr, shm->size, NX_VMSPACE_SHAREMEM) != NX_EOK)
    {
        return NX_EFAULT;
    }

    NX_ASSERT(NX_AtomicGet(&shm->reference) > 0);

    NX_AtomicDec(&shm->reference);

    if (NX_AtomicGet(&shm->reference) == 0) /* destroy share memory */
    {
        err = NX_ShareMemClose(shm);
    }
    return err;
}
