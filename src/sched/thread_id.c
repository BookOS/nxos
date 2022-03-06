/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: NX_Thread ID for NXOS
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-7      JasonHu           Init
 */

#include <sched/thread_id.h>
#include <mm/alloc.h>
#include <xbook/debug.h>
#include <utils/memory.h>

NX_PRIVATE struct NX_ThreadID ThreadIdObject;

int NX_ThreadIdAlloc(void)
{
    NX_UArch level;
    NX_SpinLockIRQ(&ThreadIdObject.idLock, &level);

    NX_U32 nextID = ThreadIdObject.nextID;
    do 
    {
        NX_U32 idx = nextID / 32;
        NX_U32 odd = nextID % 32;
        if (!(ThreadIdObject.maps[idx] & (1 << odd)))
        {
            /* mark id used */
            ThreadIdObject.maps[idx] |= (1 << odd);
            /* set next id */
            ThreadIdObject.nextID = (nextID + 1) % NX_MAX_THREAD_NR;
            break;
        }
        nextID = (nextID + 1) % NX_MAX_THREAD_NR;
    } while (nextID != ThreadIdObject.nextID);

    /* nextID == ThreadIdObject.nextID means no id free */
    int id = (nextID != ThreadIdObject.nextID) ? nextID : -1;
    NX_SpinUnlockIRQ(&ThreadIdObject.idLock, level);
    return id;
}

void NX_ThreadIdFree(int id)
{
    if (id < 0 || id >= NX_MAX_THREAD_NR)
    {
        return;
    }
    
    NX_UArch level;
    NX_SpinLockIRQ(&ThreadIdObject.idLock, &level);
    NX_U32 idx = id / 32;
    NX_U32 odd = id % 32;
    NX_ASSERT(ThreadIdObject.maps[idx] & (1 << odd));
    ThreadIdObject.maps[idx] &= ~(1 << odd);   /* clear id */
    NX_SpinUnlockIRQ(&ThreadIdObject.idLock, level);
}

void NX_ThreadsInitID(void)
{
    ThreadIdObject.maps = NX_MemAlloc(NX_MAX_THREAD_NR / 8);
    NX_ASSERT(ThreadIdObject.maps != NX_NULL);
    NX_MemZero(ThreadIdObject.maps, NX_MAX_THREAD_NR / 8);
    ThreadIdObject.nextID = 0;
    NX_SpinInit(&ThreadIdObject.idLock);
}
