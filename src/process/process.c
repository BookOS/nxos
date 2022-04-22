/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Process
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-8       JasonHu           Init
 */

#include <process/process.h>
#include <arch/process.h>
#include <sched/thread.h>
#include <mm/alloc.h>
#include <xbook/debug.h>
#include <utils/memory.h>
#include <utils/string.h>
#include <mm/mmu.h>
#include <mm/page.h>
#include <arch/mmu.h>

#define NX_LOG_NAME "process"
#define NX_LOG_LEVEL NX_LOG_WARNING
#include <utils/log.h>

#include <xbook/debug.h>
#include <process/elf.h>
#include <fs/vfs.h>
#include <xbook/debug.h>
#include <process/uaccess.h>
#include <sched/sched.h>

NX_PRIVATE NX_Error NX_ProcessWait(NX_Process * process, int *retCode);

NX_PRIVATE void ProcessAppendThread(NX_Process *process, NX_Thread *thread)
{
    NX_UArch level;
    NX_SpinLockIRQ(&process->lock, &level);
    NX_AtomicInc(&process->threadCount);
    NX_ListAdd(&thread->processList, &process->threadPoolListHead);
    thread->resource.process = process;
    NX_SpinUnlockIRQ(&process->lock, level);
}

NX_PRIVATE void ProcessDeleteThread(NX_Process *process, NX_Thread *thread)
{
    NX_UArch level;
    NX_SpinLockIRQ(&process->lock, &level);
    NX_AtomicDec(&process->threadCount);
    NX_ListDel(&thread->processList);
    thread->resource.process = NX_NULL;
    NX_SpinUnlockIRQ(&process->lock, level);
}

NX_PRIVATE NX_Process *NX_ProcessCreateObject(NX_U32 flags)
{
    NX_Process *process = NX_MemAlloc(sizeof(NX_Process));
    if (process == NX_NULL)
    {
        return NX_NULL;
    }

    NX_VfsFileTable *ft = NX_MemAlloc(sizeof(NX_VfsFileTable));
    if (ft == NX_NULL)
    {
        NX_MemFree(process);
        return NX_NULL;
    }

    if(NX_VmspaceInit(&process->vmspace,
        NX_USER_SPACE_VADDR,
        NX_USER_SPACE_TOP,
        NX_USER_IMAGE_VADDR,
        NX_USER_IMAGE_TOP,
        NX_USER_HEAP_VADDR,
        NX_USER_HEAP_TOP,
        NX_USER_MAP_VADDR,
        NX_USER_MAP_TOP,
        NX_USER_STACK_VADDR,
        NX_USER_STACK_TOP) != NX_EOK)
    {
        NX_MemFree(ft);
        NX_MemFree(process);
        return NX_NULL;
    }

    if (NX_ProcessInitUserSpace(process, NX_USER_SPACE_VADDR, NX_USER_SPACE_SIZE) != NX_EOK)
    {
        NX_MemFree(ft);
        NX_MemFree(process);
        return NX_NULL;
    }
    
    NX_VfsFileTableInit(ft);
    process->fileTable = ft;
    process->flags = flags;

    NX_AtomicSet(&process->threadCount, 0);
    NX_ListInit(&process->threadPoolListHead);

    NX_SpinInit(&process->lock);

    process->exitCode = 0;
    process->waitExitCode = 0;

    NX_SemaphoreInit(&process->waiterSem, 0);
    NX_AtomicSet(&process->waiterNumber, 0);

    process->pid = 0;

    return process;
}

NX_Error NX_ProcessDestroyObject(NX_Process *process)
{
    if (process == NX_NULL)
    {
        return NX_EINVAL;
    }
    
    /* exit vmspace */
    NX_ASSERT(NX_VmspaceExit(&process->vmspace) == NX_EOK);
    
    /* TODO: free file table entry */
    NX_MemFree(process->fileTable);
    NX_MemFree(process);
    return NX_EOK;
}

NX_PRIVATE void ProcessThreadEntry(void *arg)
{
    NX_Thread *thread = NX_ThreadSelf();
    NX_LOG_I("Process %s/%d running...", thread->name, thread->tid);
    /* Jump into userspace to run app */
    NX_ProcessExecuteUser((void *)NX_USER_IMAGE_VADDR, (void *)NX_USER_STACK_TOP, thread->stackBase + thread->stackSize, NX_NULL);
}

NX_PRIVATE NX_Error NX_LoadCheckMachine(NX_U16 machineType)
{
    NX_U16 type = EM_NONE;
    char *machineStr = "None";

#if defined(__I386__)
    type = EM_386;
    machineStr = "Intel 80386";
#elif defined(__AMD64__)
    type = EM_X86_64;
    machineStr = "AMD x86-64 architecture";
#elif defined(__ARM__)
    type = EM_ARM;
    machineStr = "ARM";
#elif defined(__ARM64__)
    type = EM_AARCH64;
    machineStr = "ARM AARCH64";
#elif defined(__RISCV32__) || defined(__RISCV64__)
    type = EM_RISCV;
    machineStr = "RISC-V";
#endif

    if (machineType != type)
    {
        NX_LOG_E("elf file not a %s format !", machineStr);
        return NX_EINVAL;
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error NX_LoadCheck(char *path, int fd, NX_Size len, NX_Vmspace *space, Elf_Ehdr *elfHeader)
{
    NX_U8 elfMagic[SELFMAG];
    if (len < sizeof(Elf_Ehdr))
    {
        NX_LOG_E("file %s too small.", path);
        return NX_EINVAL;
    }

    /* check elf magic */
    if (NX_VfsFileSeek(fd, 0, NX_VFS_SEEK_SET, NX_NULL) != 0)
    {
        return NX_EFAULT;
    }

    if (NX_VfsRead(fd, elfMagic, sizeof(elfMagic), NX_NULL) != sizeof(elfMagic))
    {
        return NX_EIO;
    }

    if (elfMagic[EI_MAG0] != ELFMAG0 || elfMagic[EI_MAG1] != ELFMAG1 ||
        elfMagic[EI_MAG2] != ELFMAG2 || elfMagic[EI_MAG3] != ELFMAG3)
    {
        NX_LOG_E("file %s not a elf format file!", path);
        return NX_EINVAL;
    }

    /* load header */
    if (NX_VfsFileSeek(fd, 0, NX_VFS_SEEK_SET, NX_NULL) != 0)
    {
        return NX_EFAULT;
    }
    if (NX_VfsRead(fd, elfHeader, sizeof(Elf_Ehdr), NX_NULL) != sizeof(Elf_Ehdr))
    {
        return NX_EIO;
    }

    NX_LOG_D("elf ident: class %d, version %d, entry %p, machine %d", elfHeader->e_ident[EI_CLASS], elfHeader->e_ident[EI_VERSION], elfHeader->e_entry, elfHeader->e_machine);

    if (NX_LoadCheckMachine(elfHeader->e_machine) != NX_EOK)
    {
        return NX_EINVAL;
    }

    /* check ident */
#ifdef CONFIG_NX_CPU_64BITS
    if (elfHeader->e_ident[EI_CLASS] != 2) /* must be 64 bit elf header */
    {
        NX_LOG_E("elf file %s not 64 bit!", path);
        return NX_EINVAL;
    }
#else
    if (elfHeader->e_ident[EI_CLASS] != 1) /* must be 32 bit elf header */
    {
        NX_LOG_E("elf file %s not 32 bit!", path);
        return NX_EINVAL;
    }
#endif

    if (elfHeader->e_ident[EI_VERSION] != 1)
    {
        NX_LOG_E("elf file %s version must be 1!", path);
        return NX_EINVAL;
    }

    /* check entry point */
    if (elfHeader->e_entry != space->imageStart)
    {
        NX_LOG_E("elf file %s entry %p is invalid!", path, elfHeader->e_entry);
        return NX_EINVAL;
    }

    return NX_EOK;
}


NX_PRIVATE NX_Error NX_LoadMapUserSpace(int fd, NX_Size len, NX_Vmspace *space, Elf_Ehdr *elfHeader)
{
    Elf_Phdr progHeader;
    NX_Offset off = 0;
    int i;

    NX_LOG_D("elf program header info: numbers %d, off %p", elfHeader->e_phnum, elfHeader->e_phoff);

    /* if map failed, no need unmap the mapped space, because process destroy will unmap it. */
    off = elfHeader->e_phoff;
    for (i = 0; i < elfHeader->e_phnum; i++, off += sizeof(progHeader))
    {
        if (off > len)
        {
            return NX_EFAULT;
        }

        if (NX_VfsFileSeek(fd, off, NX_VFS_SEEK_SET, NX_NULL) != off)
        {
            return NX_EFAULT;
        }
        if (NX_VfsRead(fd, &progHeader, sizeof(progHeader), NX_NULL) != sizeof(progHeader))
        {
            return NX_EIO;
        }

        NX_LOG_D("elf program header[%d]: type %d, vaddr %p, file sz %p, mem sz %p", i, progHeader.p_type, progHeader.p_vaddr, progHeader.p_filesz, progHeader.p_memsz);

        if (progHeader.p_type == PT_LOAD)
        {
            NX_Size size = NX_MAX(progHeader.p_filesz, progHeader.p_memsz);
            NX_Addr addr = (NX_Addr)progHeader.p_vaddr;
            void *mappedAddr;

            NX_LOG_D("elf programe header[%d] map addr %p size %p", i, addr, size);

            if (NX_VmspaceMap(space, addr, size, NX_PAGE_ATTR_USER, 0, &mappedAddr) != NX_EOK)
            {
                return NX_ENOMEM;
            }
            if ((NX_Addr)mappedAddr != addr)
            {
                return NX_EFAULT;
            }
        }
    }
    return NX_EOK;
}

NX_PRIVATE NX_Error NX_LoadFileData(int fd, NX_Size len, NX_Vmspace *space, Elf_Ehdr *elfHeader)
{
    Elf_Phdr progHeader;
    NX_Offset progOff;
    NX_Size size;
    NX_Addr vaddr;
    NX_Addr paddr;
    NX_Addr vaddrSelf;
    NX_Size chunk;
    NX_Size readLen;
    NX_Size memSize = 0;
    int i;
    
    /* load segment data */
    progOff = elfHeader->e_phoff;
    for (i = 0; i < elfHeader->e_phnum; i++, progOff += sizeof(progHeader))
    {
        if (progOff > len)
        {
            return NX_EFAULT;
        }

        /* read program header */
        if (NX_VfsFileSeek(fd, progOff, NX_VFS_SEEK_SET, NX_NULL) != progOff)
        {
            return NX_EFAULT;
        }
        if (NX_VfsRead(fd, &progHeader, sizeof(progHeader), NX_NULL) != sizeof(progHeader))
        {
            return NX_EIO;
        }

        if (progHeader.p_type == PT_LOAD)
        {
            if (progHeader.p_filesz > progHeader.p_memsz)
            {
                NX_LOG_E("elf program header[%d]: file size %p large than mem size %p !", i, progHeader.p_filesz, progHeader.p_memsz);
                return NX_ERROR;
            }

            /* read program data */
            if (progHeader.p_offset > len)
            {
                return NX_ERROR;
            }

            if (NX_VfsFileSeek(fd, progHeader.p_offset, NX_VFS_SEEK_SET, NX_NULL) != progHeader.p_offset)
            {
                return NX_EIO;
            }
            
            size = progHeader.p_filesz;
            vaddr = (NX_Addr)progHeader.p_vaddr;
            readLen = 0;

            while (size > 0)
            {
                paddr = (NX_Addr)NX_MmuVir2Phy(&space->mmu, vaddr); /* use mmu translate to phy addr */
                NX_ASSERT(paddr);
                vaddrSelf = NX_Phy2Virt(paddr); /* translate physic addr to kernel virtual addr to access */

                chunk = (size < NX_PAGE_SIZE) ? size : NX_PAGE_SIZE;

                if (NX_VfsRead(fd, (void *)vaddrSelf, chunk, NX_NULL) != chunk)
                {
                    return NX_EIO;
                }

                size -= chunk;
                vaddr += chunk;
                readLen += chunk;
            }

            if (readLen != progHeader.p_filesz)
            {
                return NX_EIO;
            }

            /* clear bss */
            if (progHeader.p_filesz < progHeader.p_memsz) /* bss area is mem size > file size */
            {
                NX_Offset bssOff;
                NX_Size bssSize = progHeader.p_memsz - progHeader.p_filesz;
                NX_LOG_D("elf program header[%d]: bss area size %p", i, bssSize);

                bssOff = progHeader.p_filesz & NX_PAGE_MASK; /* offset in page */
                vaddr = (progHeader.p_vaddr + progHeader.p_filesz) & NX_PAGE_ADDR_MASK; /* page alinged addr */
                while (bssSize > 0)
                {
                    chunk = (bssSize < (NX_PAGE_SIZE - bssOff)) ? bssSize : NX_PAGE_SIZE - bssOff;
                    paddr = (NX_Addr)NX_MmuVir2Phy(&space->mmu, vaddr);
                    NX_ASSERT(paddr);
                    vaddrSelf = NX_Phy2Virt(paddr);
                    NX_LOG_D("elf program header[%d]: bss clear vaddr %p, paddr %p chunk size %p", i, vaddr + bssOff, (void *)((NX_U8 *)vaddrSelf + bssOff), chunk);
                    NX_MemZero((void *)((NX_U8 *)vaddrSelf + bssOff), chunk);
                
                    bssOff = 0;
                    bssSize -= chunk;
                    vaddr += NX_PAGE_SIZE;
                }
            }

            memSize += progHeader.p_memsz;
        }
    }
    
    NX_LOG_D("elf used memory size %p", memSize);

    /* space resize image and heap size */
    NX_VmspaceResizeImage(space, memSize);

    return NX_EOK;
}

NX_PRIVATE NX_Error NX_LoadElf(char *path, int fd, NX_Size len, NX_Vmspace *space)
{
    if (fd < 0 || !len || !space)
    {
        return NX_EINVAL;
    }

    Elf_Ehdr elfHeader;
    NX_Error err = NX_EOK;

    err = NX_LoadCheck(path, fd, len, space, &elfHeader);
    if (err != NX_EOK)
    {
        return err;
    }

    err = NX_LoadMapUserSpace(fd, len, space, &elfHeader);
    if (err != NX_EOK)
    {
        NX_LOG_E("file %s do space map failed!", path);
        return err;
    }

    err = NX_LoadFileData(fd, len, space, &elfHeader);
    if (err != NX_EOK)
    {
        NX_LOG_E("file %s load image failed!", path);
        return err;
    }

    return NX_EOK;
}

/**
 * Load code & data for process image
 */
NX_PRIVATE NX_Error NX_ProcessLoadImage(NX_Process *process, char *path)
{
    NX_ASSERT(process->vmspace.mmu.table);
    int fd;
    NX_Error err = NX_EOK;
    NX_Offset len;
    NX_Size imageMaxSize;
    NX_Vmspace *space;

    fd = NX_VfsOpen(path, NX_VFS_O_RDONLY, 0, &err);
    if (fd < NX_EOK)
    {
        NX_LOG_E("process open file %s failed ! with error %s", path, NX_ErrorToString(err));
        return NX_ENOSRCH;
    }

    len = NX_VfsFileSeek(fd, 0, NX_VFS_SEEK_END, &err);
    if (!len || err != NX_EOK)
    {
        NX_LOG_E("file %s too small !", path);
        NX_VfsClose(fd);
        return NX_ENOSRCH;
    }

    if (NX_VfsFileSeek(fd, 0, NX_VFS_SEEK_SET, NX_NULL) != 0)
    {
        NX_LOG_E("seek file %s failed !", path);
        NX_VfsClose(fd);
        return NX_ENOSRCH;
    }

    NX_LOG_D("process execute file %s size is %d", path, len);

    space = &process->vmspace;
    imageMaxSize = space->imageEnd - space->imageStart;
    if (len > imageMaxSize)
    {
        NX_LOG_E("image too large %p than %p !", len, imageMaxSize);
        NX_VfsClose(fd);
        return NX_EFAULT;
    }

    err = NX_LoadElf(path, fd, len, space);
    if (err != NX_EOK)
    {
        NX_LOG_E("load elf %s failed with err %d!", path, err);
        NX_VfsClose(fd);
        return err;
    }

    /* close file */
    NX_ASSERT(NX_VfsClose(fd) == 0);

    return NX_EOK;
}

/**
 * launch a process with image
 */
NX_Error NX_ProcessLaunch(char *path, NX_U32 flags, int *retCode, char *cmd, char *env)
{
    NX_Vmspace *space;
    char *name;

    if (path == NX_NULL)
    {
        return NX_EINVAL;
    }

    /* check path exist */
    if (NX_VfsAccess(path, NX_VFS_R_OK) != NX_EOK)
    {
        return NX_EPERM;
    }

    /* make name */
    name = NX_StrChrReverse(path, '/');
    if (name == NX_NULL) /* no '/' */
    {
        name = path;
    }
    else
    {
        name++; /* skip '/' */
    }

    NX_Process *process = NX_ProcessCreateObject(flags);
    if (process == NX_NULL)
    {
        return NX_ENOMEM;
    }

    NX_Thread *thread = NX_ThreadCreate(name, ProcessThreadEntry, NX_NULL, NX_THREAD_PRIORITY_NORMAL);

    if (thread == NX_NULL)
    {
        NX_ProcessDestroyObject(process);
        return NX_ENOMEM;
    }

    if (NX_ProcessLoadImage(process, path) != NX_EOK)
    {
        NX_ProcessDestroyObject(process);
        NX_ThreadDestroy(thread);
        return NX_ENORES;
    }

    space = &process->vmspace;
    /* map user stack */
    if (NX_VmspaceMap(space, space->stackEnd - NX_PROCESS_USER_SATCK_SIZE, NX_PROCESS_USER_SATCK_SIZE, NX_PAGE_ATTR_USER, 0, NX_NULL) != NX_EOK)
    {
        NX_ASSERT(NX_VmspaceUnmap(space, space->imageStart, space->imageEnd - space->imageStart) == NX_EOK);
        NX_ProcessDestroyObject(process);
        NX_ThreadDestroy(thread);
        return NX_ENOMEM;
    }

    ProcessAppendThread(process, thread);

    /* override default file table */
    NX_ThreadSetFileTable(thread, process->fileTable);
    process->pid = thread->tid;

    if (NX_ThreadStart(thread) != NX_EOK)
    {
        NX_ASSERT(NX_VmspaceUnmap(space, space->stackEnd - NX_PROCESS_USER_SATCK_SIZE, NX_PROCESS_USER_SATCK_SIZE) == NX_EOK);
        NX_ASSERT(NX_VmspaceUnmap(space, space->imageStart, space->imageEnd - space->imageStart) == NX_EOK);
        NX_ProcessDestroyObject(process);
        NX_ThreadDestroy(thread);
        return NX_EFAULT;
    }
    
    if (process->flags & NX_PROC_FLAG_WAIT)
    {
        return NX_ProcessWait(process, retCode);
    }

    return NX_EOK;
}

void NX_ProcessExit(int exitCode)
{
    NX_Thread *thread, *next;
    NX_Thread *cur = NX_ThreadSelf();
    NX_Process *process = cur->resource.process;
    NX_ASSERT(process != NX_NULL);
    
    process->exitCode = exitCode;

    /* terminate all thread on list */
    NX_ListForEachEntrySafe(thread, next, &process->threadPoolListHead, processList)
    {
        if (thread != cur)
        {
            NX_ThreadTerminate(thread);
        }
    }

    /* wait other threads exit done */
    while (NX_AtomicGet(&process->threadCount) > 1)
    {
        NX_ThreadYield();
    }

    /* exit this thread */
    NX_ThreadExit();
    NX_PANIC("NX_ProcessExit should never be here!");
}

NX_PRIVATE void ProcessExitNotify(NX_Process * process)
{
    /* copy return value */
    NX_Thread * waiterThread;

    NX_ListForEachEntry(waiterThread, &process->waiterSem.semWaitList, blockList)
    {
        NX_ASSERT(waiterThread->resource.process);
        waiterThread->resource.process->waitExitCode = process->exitCode;
    }

    /* wakeup waiter */
    NX_IArch waiters = NX_AtomicGet(&process->waiterNumber);

    while (waiters-- > 0)
    {
        NX_SemaphoreSignal(&process->waiterSem);
    }
}

void NX_ThreadExitProcess(NX_Thread *thread, NX_Process *process)
{
    NX_ASSERT(process != NX_NULL && thread != NX_NULL);
    ProcessDeleteThread(process, thread);

    if (NX_AtomicGet(&process->threadCount) == 0)
    {
        /* thread exit need to free process in the last */
        thread->resource.process = process;

        ProcessExitNotify(process);
    }
}

NX_PRIVATE NX_Error NX_ProcessWait(NX_Process * process, int *retCode)
{
    NX_Thread * self;
    
    if (process == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    self = NX_ThreadSelf();

    NX_AtomicInc(&process->waiterNumber);
    /* wait on waiters sem */
    NX_SemaphoreWait(&process->waiterSem);
    if (retCode)
    {
        *retCode = self->resource.process->waitExitCode;
    }

    return NX_EOK;
}
