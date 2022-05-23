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

#include <base/process.h>
#include <arch/process.h>
#include <base/thread.h>
#include <base/malloc.h>
#include <base/debug.h>
#include <base/memory.h>
#include <base/string.h>
#include <base/mmu.h>
#include <base/page.h>
#include <arch/mmu.h>

#define NX_LOG_NAME "process"
#define NX_LOG_LEVEL NX_LOG_WARNING
#include <base/log.h>

#include <base/debug.h>
#include <base/elf.h>
#include <base/vfs.h>
#include <base/debug.h>
#include <base/uaccess.h>
#include <base/sched.h>
#include <base/env.h>

NX_PRIVATE NX_Error NX_ProcessWait(NX_Process * process, NX_U32 *exitCode);

void NX_ProcessAppendThread(NX_Process *process, NX_Thread *thread)
{
    NX_UArch level;
    NX_SpinLockIRQ(&process->lock, &level);
    NX_AtomicInc(&process->threadCount);
    NX_ListAdd(&thread->processList, &process->threadPoolListHead);
    thread->resource.process = process;
    NX_SpinUnlockIRQ(&process->lock, level);
}

void NX_ProcessDeleteThread(NX_Process *process, NX_Thread *thread)
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
    
    if (NX_ExposedObjectTableInit(&process->exobjTable, NX_EXOBJ_DEFAULT_NR) != NX_EOK)
    {
        NX_VmspaceExit(&process->vmspace);
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

    process->pid = 0;
    process->parentPid = 0;
    
    process->args = NX_NULL;
    NX_MemZero(process->cwd, sizeof(process->cwd));
    NX_StrCopy(process->cwd, NX_PROCESS_CWD_DEFAULT);

    return process;
}

NX_Error NX_ProcessDestroyObject(NX_Process *process)
{
    if (process == NX_NULL)
    {
        return NX_EINVAL;
    }
    
    NX_ExposedObjectTableExit(&process->exobjTable);

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
    NX_Process * process = thread->resource.process;
    NX_LOG_I("Process %s/%d running...", thread->name, thread->tid);
    /* Jump into userspace to run app */
    NX_ProcessExecuteUser((void *)NX_USER_IMAGE_VADDR, (void *)NX_PAGE_ALIGNDOWN((NX_Addr)process->args),
                          thread->stackBase + thread->stackSize, process->args);
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

NX_PRIVATE NX_Error ProcessMapStack(NX_Process *process, char *cmd, char *env)
{
    NX_Vmspace * space = NX_NULL;
    NX_Size stackSize = NX_PROCESS_USER_SATCK_SIZE;
    NX_Size cmdLen = 0;
    NX_Size envLen = 0;
    NX_Size argLen = 0;
    NX_Size pageCount;
    NX_Addr cmdStackAddr;
    NX_Addr envStackAddr;
    NX_Addr argsStackAddr;
    void *args[NX_PROCESS_ARGS + 1] = {0,};
    
    space = &process->vmspace;

    /* clac stack args len */
    if (cmd)
    {
        cmdLen = NX_ALIGN_UP(NX_StrLen(cmd) + 1, sizeof(NX_Addr));
    }
    else
    {
        cmdLen = sizeof(NX_Addr);
    }

    if (env)
    {        
        envLen = NX_ALIGN_UP(NX_StrLen(env) + 1, sizeof(NX_Addr));
    }
    else
    {
        envLen = sizeof(NX_Addr);
    }

    argLen = sizeof(args);

    /* calc stack size */
    stackSize += cmdLen; /* reserve cmd size */
    stackSize += envLen; /* reserve env size */

    stackSize += argLen; /* reserve args array size */

    pageCount = NX_DIV_ROUND_UP(stackSize, NX_PAGE_SIZE); /* calc stack page size */

    envStackAddr = space->stackEnd - envLen;
    cmdStackAddr = envStackAddr - cmdLen;
    argsStackAddr = cmdStackAddr - argLen;

    /* make args */
    args[0] = (void *)cmdStackAddr;
    args[1] = (void *)envStackAddr;

    /* check stack */
    space->stackBottom = space->stackEnd - pageCount * NX_PAGE_SIZE;
    if (space->stackBottom < space->stackStart) /* stack overflow! */
    {
        NX_LOG_E("stack overflow! pages %d needed, stack too big, please check you cmd or env.", pageCount);
        return NX_ENOMEM;
    }

    /* map user stack */
    space = &process->vmspace;
    if (NX_VmspaceMap(space, space->stackBottom, pageCount * NX_PAGE_SIZE, NX_PAGE_ATTR_USER, 0, NX_NULL) != NX_EOK)
    {
        return NX_ENOMEM;
    }

    /* copy stack */
    if (NX_VmspaceWrite(space, (char *)argsStackAddr, (char *)args, argLen))
    {
        goto copyFailed;
    }

    if (cmd)
    {
        if (NX_VmspaceWrite(space, (char *)cmdStackAddr, cmd, cmdLen))
        {
            goto copyFailed;
        }
    }

    if (env)
    {
        if (NX_VmspaceWrite(space, (char *)envStackAddr, env, envLen))
        {
            goto copyFailed;
        }
    }

    /* save args on the stack */
    process->args = (void *)argsStackAddr;

    return NX_EOK;

copyFailed:
    NX_VmspaceUnmap(space, space->stackBottom, pageCount * NX_PAGE_SIZE);
    return NX_EFAULT;
}

NX_Error NX_ProcessMapTls(NX_Process * process, NX_Thread * thread)
{
    NX_Vmspace * space = NX_NULL;
    void * tlsAddr = NX_NULL;
    NX_TlsArea tlsArea = {NX_NULL, NX_EOK};

    space = &process->vmspace;
    if (NX_VmspaceMap(space, 0, NX_PROCESS_TLS_SIZE, NX_PAGE_ATTR_USER, 0, &tlsAddr) != NX_EOK)
    {
        return NX_ENOMEM;
    }

    tlsArea.tlsAreaSelf = tlsAddr;

    /* copy tls addr to tls area */
    if (NX_VmspaceWrite(space, (char *)tlsAddr, (char *)&tlsArea, sizeof(tlsArea)) != NX_EOK)
    {
        NX_VmspaceUnmap(space, (NX_Addr)tlsAddr, NX_PROCESS_TLS_SIZE);
        return NX_EFAULT;
    }

    thread->resource.tls = tlsAddr;

    return NX_EOK;
}

NX_Error NX_ProcessUnmapTls(NX_Process * process, NX_Thread * thread)
{
    NX_Vmspace * space = NX_NULL;

    if (thread->resource.tls == NX_NULL)
    {
        return NX_ENORES;
    }

    space = &process->vmspace;
    if (NX_VmspaceUnmap(space, (NX_Addr)thread->resource.tls, NX_PROCESS_TLS_SIZE) != NX_EOK)
    {
        return NX_ENOMEM;
    }

    thread->resource.tls = NX_NULL;

    return NX_EOK;
}

NX_PRIVATE NX_Error ProcessUnmapStack(NX_Process *process)
{
    NX_Vmspace *space;
    space = &process->vmspace;

    return NX_VmspaceUnmap(space, space->stackBottom, space->stackEnd - space->stackBottom);
}

NX_PRIVATE NX_Error SearchInEnv(char * path, char * absPath, char *env)
{
    char * array[NX_PORCESS_ENV_ARGS + 1];
    int argc;
    int i;
    char * p;
    char * tmpEnv;

    tmpEnv = NX_StrDup(env);
    if (tmpEnv == NX_NULL)
    {
        return NX_ENOMEM;
    }

    argc = NX_EnvToArray(tmpEnv, array, NX_PORCESS_ENV_ARGS);
    if (argc > 0)
    {
        for (i = 0; i < argc; i++)
        {
            p = array[i];
            NX_ASSERT(*p);

            if (*p != '/')
            {
                continue;
            }

            /* abs path = env + / + path */
            NX_MemZero(absPath, NX_VFS_MAX_PATH);

            NX_StrCat(absPath, p);
            if (absPath[NX_StrLen(absPath) - 1] != '/')
            {
                NX_StrCat(absPath, "/");
            }
            NX_StrCat(absPath, path);

            if (NX_VfsAccess(absPath, NX_VFS_R_OK) == NX_EOK)
            {
                NX_MemFree(tmpEnv);
                return NX_EOK;
            }
        }
    }

    NX_MemFree(tmpEnv);
    return NX_ENOSRCH;
}

/**
 * launch a process with image
 */
NX_Error NX_ProcessLaunch(char *path, NX_U32 flags, NX_U32 *exitCode, char *cmd, char *env)
{
    NX_Thread * self;
    NX_Vmspace *space;
    char *name;
    NX_Solt threadSolt = NX_SOLT_INVALID_VALUE;
    char absPath[NX_VFS_MAX_PATH] = {0,};

    if (path == NX_NULL)
    {
        return NX_EINVAL;
    }

    NX_CopyFromUser(absPath, path, NX_StrLen(path));

    /* check path exist */
    if (NX_VfsAccess(absPath, NX_VFS_R_OK) != NX_EOK)
    {
        if (*absPath == '/') /* abs path means no res */
        {
            return NX_ENOSRCH;
        }

        if (SearchInEnv(path, absPath, env) != NX_EOK)
        {
            return NX_ENOSRCH;
        }
    }

    /* make name */
    name = NX_StrChrReverse(absPath, '/');
    if (name == NX_NULL) /* no '/' */
    {
        name = absPath;
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

    if (NX_ProcessLoadImage(process, absPath) != NX_EOK)
    {
        NX_ProcessDestroyObject(process);
        NX_ThreadDestroy(thread);
        return NX_ENORES;
    }

    space = &process->vmspace;

    if (ProcessMapStack(process, cmd, env) != NX_EOK)
    {
        NX_ASSERT(NX_VmspaceUnmap(space, space->imageStart, space->imageEnd - space->imageStart) == NX_EOK);
        NX_ProcessDestroyObject(process);
        NX_ThreadDestroy(thread);
        return NX_ENOMEM;
    }

    /* map tls */
    if (NX_ProcessMapTls(process, thread) != NX_EOK)
    {
        ProcessUnmapStack(process);
        NX_ASSERT(NX_VmspaceUnmap(space, space->imageStart, space->imageEnd - space->imageStart) == NX_EOK);
        NX_ProcessDestroyObject(process);
        NX_ThreadDestroy(thread);
        return NX_ENOMEM;
    }

    NX_ProcessAppendThread(process, thread);

    /* override default file table */
    NX_ThreadSetFileTable(thread, process->fileTable);
    process->pid = thread->tid;

    self = NX_ThreadSelf();
    if (self->resource.process)
    {
        process->parentPid = self->resource.process->pid; /* set current process as parent */
    }

    /* copy execute path */
    NX_StrCopyN(process->exePath, absPath, sizeof(process->exePath));

    NX_ASSERT(NX_ProcessInstallSolt(process, thread, NX_EXOBJ_THREAD, NX_NULL, &threadSolt) == NX_EOK);

    if (NX_ThreadStart(thread) != NX_EOK)
    {
        NX_ProcessUninstallSolt(process, threadSolt);
        NX_ProcessUnmapTls(process, thread);
        ProcessUnmapStack(process);
        NX_ASSERT(NX_VmspaceUnmap(space, space->imageStart, space->imageEnd - space->imageStart) == NX_EOK);
        NX_ProcessDestroyObject(process);
        NX_ThreadDestroy(thread);
        return NX_EFAULT;
    }
    
    if (process->flags & NX_PROC_FLAG_WAIT)
    {
        return NX_ProcessWait(process, exitCode);
    }

    return NX_EOK;
}

void NX_ProcessExit(NX_U32 exitCode)
{
    NX_UArch level;
    NX_Thread *thread, *next;
    NX_Thread *cur = NX_ThreadSelf();
    NX_Process *process = cur->resource.process;
    NX_ASSERT(process != NX_NULL);
    
    process->exitCode = exitCode;

    NX_SpinLockIRQ(&process->lock, &level);
    /* terminate all thread on list */
    NX_ListForEachEntrySafe(thread, next, &process->threadPoolListHead, processList)
    {
        if (thread != cur)
        {
            NX_ThreadTerminate(thread, exitCode);
        }
    }

    NX_SpinUnlockIRQ(&process->lock, level);
    
    /* wait other threads exit done */
    while (NX_AtomicGet(&process->threadCount) > 1)
    {
        NX_ThreadYield();
    }

    /* exit this thread */
    NX_ThreadExit(exitCode);
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
    NX_SemaphoreSignalAll(&process->waiterSem);
}

void NX_ThreadExitProcess(NX_Thread *thread, NX_Process *process)
{
    NX_Solt solt;

    NX_ASSERT(process != NX_NULL && thread != NX_NULL);
    NX_ProcessDeleteThread(process, thread);

    /* uninstall thread solt in process */
    solt = NX_ProcessLocateSolt(process, thread, NX_EXOBJ_THREAD);
    if (solt != NX_SOLT_INVALID_VALUE)
    {
        NX_ProcessUninstallSolt(process, solt);
    }

    /* unmap user stack */
    if (thread->userStackBase)
    {
        NX_VmspaceUnmap(&process->vmspace, (NX_Addr)thread->userStackBase, thread->userStackSize);
    }

    /* unmap tls */
    NX_ProcessUnmapTls(process, thread);

    if (NX_AtomicGet(&process->threadCount) == 0)
    {
        /* thread exit need to free process in the last */
        thread->resource.process = process;

        ProcessExitNotify(process);
    }
}

NX_PRIVATE NX_Error NX_ProcessWait(NX_Process * process, NX_U32 *exitCode)
{
    NX_Thread * self;
    
    if (process == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    self = NX_ThreadSelf();

    if (self->resource.process == process)
    {
        NX_LOG_E("process %d wait self!", process->pid);
        return NX_EINVAL;
    }

    /* wait on waiters sem */
    NX_SemaphoreWait(&process->waiterSem);
    if (exitCode)
    {
        *exitCode = self->resource.process->waitExitCode;
    }

    return NX_EOK;
}

char * NX_ProcessGetCwd(NX_Process * process)
{
    if (!process)
    {
        return NX_NULL;
    }
    return &process->cwd[0];
}

NX_Error NX_ProcessSetCwd(NX_Process * process, const char * path)
{
    NX_Error err;
    NX_VfsStatInfo st;
    char absPath[NX_VFS_MAX_PATH + 1] = {0};

    if (!process || !path)
    {
        return NX_EINVAL;
    }

    if (NX_StrLen(path) >= NX_VFS_MAX_PATH)
    {
        return NX_EINVAL;
    }

    if ((err = NX_VfsBuildAbsPath(path, absPath)) != NX_EOK)
    {
        return err;
    }

    if (NX_VfsStat(absPath, &st) != NX_EOK)
    {
        return NX_ENOSRCH;
    }

    if (!NX_VFS_S_ISDIR(st.mode)) /* not dir */
    {
        return NX_EPERM;
    }

    NX_StrCopyN(process->cwd, absPath, NX_VFS_MAX_PATH);
    return NX_EOK;
}
