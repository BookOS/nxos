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

#include <base/thread_id.h>
#include <base/malloc.h>
#include <base/debug.h>
#include <base/memory.h>

NX_PRIVATE struct NX_ThreadID threadIdObject;

int NX_ThreadIdAlloc(void)
{
    NX_UArch level;
    NX_SpinLockIRQ(&threadIdObject.idLock, &level);

    NX_U32 nextID = threadIdObject.nextID;
    do 
    {
        NX_U32 idx = nextID / 32;
        NX_U32 odd = nextID % 32;
        if (!(threadIdObject.maps[idx] & (1 << odd)))
        {
            /* mark id used */
            threadIdObject.maps[idx] |= (1 << odd);
            /* set next id */
            threadIdObject.nextID = (nextID + 1) % NX_MAX_THREAD_NR;
            break;
        }
        nextID = (nextID + 1) % NX_MAX_THREAD_NR;
    } while (nextID != threadIdObject.nextID);

    /* nextID == threadIdObject.nextID means no id free */
    int id = (nextID != threadIdObject.nextID) ? nextID : -1;
    NX_SpinUnlockIRQ(&threadIdObject.idLock, level);
    return id;
}

void NX_ThreadIdFree(int id)
{
    if (id < 0 || id >= NX_MAX_THREAD_NR)
    {
        return;
    }
    
    NX_UArch level;
    NX_SpinLockIRQ(&threadIdObject.idLock, &level);
    NX_U32 idx = id / 32;
    NX_U32 odd = id % 32;
    NX_ASSERT(threadIdObject.maps[idx] & (1 << odd));
    threadIdObject.maps[idx] &= ~(1 << odd);   /* clear id */
    NX_SpinUnlockIRQ(&threadIdObject.idLock, level);
}

void NX_ThreadsInitID(void)
{
    threadIdObject.maps = NX_MemAlloc(NX_MAX_THREAD_NR / 8);
    NX_ASSERT(threadIdObject.maps != NX_NULL);
    NX_MemZero(threadIdObject.maps, NX_MAX_THREAD_NR / 8);
    threadIdObject.nextID = 0;
    NX_SpinInit(&threadIdObject.idLock);
}
