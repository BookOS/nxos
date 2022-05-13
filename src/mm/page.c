/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Page alloctor
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-18     JasonHu           Init
 */

#include <mm/buddy.h>
#include <mm/page.h>
#include <utils/log.h>
#include <xbook/debug.h>
#include <sched/spin.h>

NX_PRIVATE NX_BuddySystem *BuddySystemArray[NX_PAGE_ZONE_NR]; 

NX_PRIVATE NX_Spin buddyLock[NX_PAGE_ZONE_NR];

/**
 * Init buddy memory allocator
 */
void NX_PageInitZone(NX_PageZone zone, void *mem, NX_Size size)
{
    NX_ASSERT(zone >= NX_PAGE_ZONE_NORMAL && zone < NX_PAGE_ZONE_NR && size > 0);
    BuddySystemArray[zone] = NX_BuddyCreate(mem, size);
    NX_ASSERT(BuddySystemArray[zone] != NX_NULL);
    NX_SpinInit(&buddyLock[zone]);
}

void *NX_PageAllocInZone(NX_PageZone zone, NX_Size count)
{
    NX_ASSERT(zone >= NX_PAGE_ZONE_NORMAL && zone < NX_PAGE_ZONE_NR && count > 0);
    void *addr;
    NX_UArch level;
    NX_SpinLockIRQ(&buddyLock[zone], &level);
    addr = NX_BuddyAllocPage(BuddySystemArray[zone], count);
    NX_SpinUnlockIRQ(&buddyLock[zone], level);
    return addr;
}

NX_Error NX_PageFreeInZone(NX_PageZone zone, void *ptr)
{
    NX_ASSERT(zone >= NX_PAGE_ZONE_NORMAL && zone < NX_PAGE_ZONE_NR && ptr != NX_NULL);
    NX_Error err;
    NX_UArch level;
    NX_SpinLockIRQ(&buddyLock[zone], &level);
    err = NX_BuddyFreePage(BuddySystemArray[zone], ptr);
    NX_SpinUnlockIRQ(&buddyLock[zone], level);
    return err;
}

NX_Error NX_PageIncreaseInZone(NX_PageZone zone, void *ptr)
{
    NX_ASSERT(zone >= NX_PAGE_ZONE_NORMAL && zone < NX_PAGE_ZONE_NR && ptr != NX_NULL);
    NX_Error err;
    NX_UArch level;
    NX_SpinLockIRQ(&buddyLock[zone], &level);
    err = NX_BuddyIncreasePage(BuddySystemArray[zone], ptr);
    NX_SpinUnlockIRQ(&buddyLock[zone], level);
    return err;
}

void *NX_PageZoneGetBase(NX_PageZone zone)
{
    NX_ASSERT(zone >= NX_PAGE_ZONE_NORMAL && zone < NX_PAGE_ZONE_NR);
    void *addr;
    NX_UArch level;
    NX_SpinLockIRQ(&buddyLock[zone], &level);
    addr = BuddySystemArray[zone]->pageStart;
    NX_SpinUnlockIRQ(&buddyLock[zone], level);
    return addr;
}

NX_Size NX_PageZoneGetPages(NX_PageZone zone)
{
    NX_ASSERT(zone >= NX_PAGE_ZONE_NORMAL && zone < NX_PAGE_ZONE_NR);
    NX_Size size;
    NX_UArch level;
    NX_SpinLockIRQ(&buddyLock[zone], &level);
    size = (BuddySystemArray[zone]->maxPFN + 1);
    NX_SpinUnlockIRQ(&buddyLock[zone], level);
    return size;
}

void *NX_PageZoneGetBuddySystem(NX_PageZone zone)
{
    NX_ASSERT(zone >= NX_PAGE_ZONE_NORMAL && zone < NX_PAGE_ZONE_NR);
    void *addr;
    NX_UArch level;
    NX_SpinLockIRQ(&buddyLock[zone], &level);
    addr = BuddySystemArray[zone];
    NX_SpinUnlockIRQ(&buddyLock[zone], level);
    return addr;
}

NX_Size NX_PageGetTotal(void)
{
    NX_Size count;
    NX_UArch level;

    NX_SpinLockIRQ(&buddyLock[NX_PAGE_ZONE_NORMAL], &level);
    count = BuddySystemArray[NX_PAGE_ZONE_NORMAL]->maxPFN + 1;
    NX_SpinUnlockIRQ(&buddyLock[NX_PAGE_ZONE_NORMAL], level);
    return count;
}

NX_Size NX_PageGetUsed(void)
{
    NX_Size count;
    NX_UArch level;

    NX_SpinLockIRQ(&buddyLock[NX_PAGE_ZONE_NORMAL], &level);
    count = BuddySystemArray[NX_PAGE_ZONE_NORMAL]->usedPage;
    NX_SpinUnlockIRQ(&buddyLock[NX_PAGE_ZONE_NORMAL], level);
    return count;
}
