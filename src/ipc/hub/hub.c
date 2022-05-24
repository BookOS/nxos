/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: hub system
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-6       JasonHu           Init
 */

#include <nxos.h>
#include <base/hub.h>
#include <base/malloc.h>
#include <base/page.h>
#include <base/string.h>
#include <base/debug.h>
#include <base/log.h>
#include <base/memory.h>
#include <base/vmspace.h>
#include <base/uaccess.h>

#define NX_HUB_CLIENTS_MAX (-1UL)

NX_PRIVATE NX_LIST_HEAD(hubSystemListHead);
NX_PRIVATE NX_SPIN_DEFINE_UNLOCKED(hubSystemLock);

NX_PRIVATE void NX_HubReleaseMdl(NX_HubChannel *channel);

void NX_HubDump(NX_Hub *hub)
{
    NX_LOG_D("------------ Dump Hub ------------");
    NX_LOG_D("hub %p, name: %s, max client: %d", hub, hub->name, hub->maxClient);
    NX_LOG_D("hub total channels: %d", 
        NX_AtomicGet(&hub->totalChannels));
    NX_LOG_D("====================================================");
}

void NX_HubChannelDump(NX_HubChannel *channel)
{
    NX_LOG_D("------------ Dump Hub Channel ----------------");
    NX_LOG_D("hub cahnnel source: %p/%s, target: %p/%s, retval: %p",
        channel->sourceThread, channel->sourceThread ? channel->sourceThread->name : "null",
        channel->targetThread, channel->targetThread ? channel->targetThread->name : "null");
    NX_LOG_D("hub cahnnel state: %d", channel->state);    
    NX_LOG_D("====================================================");
}

NX_PRIVATE void HubChannelInit(NX_HubChannel *channel,
    NX_Hub *hub)
{
	channel->state = NX_HUB_CHANNEL_IDLE;
	NX_ListInit(&channel->list);
	NX_ListInit(&channel->mdlListHead);
	channel->retVal = 0;
    channel->retErr = NX_EOK;
	channel->hub = hub;
    NX_SemaphoreInit(&channel->syncSem, 0);
}

NX_PRIVATE NX_HubChannel *CreateChannel(NX_Hub *hub)
{
	NX_HubChannel *channel = NX_MemAllocEx(NX_HubChannel);
	if (channel == NX_NULL)
	{
		return NX_NULL;
	}
	
    HubChannelInit(channel, hub);

	return channel;
}

NX_PRIVATE NX_Error DestroyChannel(NX_HubChannel *channel)
{
    NX_Hub *hub;
	NX_ASSERT(channel);

    hub = channel->hub;
    if (hub == NX_NULL)
    {
        return NX_EFAULT;
    }

	NX_MemFree(channel);
	return NX_EOK;
}

NX_PRIVATE NX_Error HubAddChannel(NX_Hub *hub, NX_HubChannel *channel)
{
	NX_UArch level;
	NX_ASSERT(hub);
	NX_ASSERT(channel);
	
	NX_SpinLockIRQ(&hub->lock, &level);
	NX_ListAdd(&channel->list, &hub->channelListHead);
	NX_AtomicInc(&hub->totalChannels);
	channel->state = NX_HUB_CHANNEL_IDLE;
	NX_SpinUnlockIRQ(&hub->lock, level);
	return NX_EOK;
}

NX_PRIVATE NX_Error HubDelChannelLocked(NX_Hub *hub, NX_HubChannel *channel)
{
	NX_ASSERT(hub);
	NX_ASSERT(channel);
	
	NX_ListDel(&channel->list);
	NX_AtomicDec(&hub->totalChannels);

	return NX_EOK;
}

NX_PRIVATE NX_Error NX_USED HubDelChannel(NX_Hub *hub, NX_HubChannel *channel)
{
	NX_UArch level;
	NX_ASSERT(hub);
	NX_ASSERT(channel);
	
	NX_SpinLockIRQ(&hub->lock, &level);
	
    HubDelChannelLocked(hub, channel);

	NX_SpinUnlockIRQ(&hub->lock, level);
	return NX_EOK;
}

NX_PRIVATE NX_HubChannel *HubPickChannel(NX_Hub *hub)
{
	NX_UArch level;
	NX_HubChannel *channel;
	NX_ASSERT(hub);
	
	NX_SpinLockIRQ(&hub->lock, &level);
	NX_ListForEachEntry(channel, &hub->channelListHead, list)
	{
		if (channel->state == NX_HUB_CHANNEL_IDLE)
		{
			channel->state = NX_HUB_CHANNEL_READY;
			NX_SpinUnlockIRQ(&hub->lock, level);
			return channel;
		}
	}
	
	NX_SpinUnlockIRQ(&hub->lock, level);
	return NX_NULL;
}

NX_PRIVATE void HubInit(NX_Hub *hub, const char *name, NX_Size maxClient)
{
	NX_StrCopy(hub->name, name);
	
	hub->maxClient = maxClient;

	NX_AtomicSet(&hub->totalChannels, 0);
	NX_ListInit(&hub->channelListHead);
	NX_ListInit(&hub->list);
	hub->idleTime = 0;
	NX_SpinInit(&hub->lock);
}

NX_PRIVATE NX_Error DestroyHub(NX_Hub *hub)
{
	NX_MemFree(hub);
	return NX_EOK;
}

NX_PRIVATE NX_Hub *CreateHub(const char *name, NX_Size maxClient)
{
    NX_Hub *hub;

	hub = NX_MemAllocEx(NX_Hub);
	if (hub == NX_NULL)
	{
		return NX_NULL;
	}
	
    HubInit(hub, name, maxClient);
	return hub;
}

NX_PRIVATE NX_Hub *SearchHub(const char *name)
{
	NX_Hub *hub;
	NX_ListForEachEntry(hub, &hubSystemListHead, list)
	{
		if (!NX_StrCmpN(hub->name, name, NX_HUB_NAME_LEN))
		{
			return hub;
		}
	}
	return NX_NULL;
}

NX_PRIVATE NX_HubChannel *GetFreeChannel(NX_Hub *hub, NX_Thread *source)
{
	NX_HubChannel *channel;
	NX_ASSERT(hub);

	channel = HubPickChannel(hub);
	
    if (channel == NX_NULL) /* try add a channel */
    {
        if (NX_AtomicGet(&hub->totalChannels) < hub->maxClient)
        {
            channel = CreateChannel(hub);
            if (channel == NX_NULL)
            {
                return NX_NULL;
            }

            HubAddChannel(hub, channel);

            channel = HubPickChannel(hub);
        }
        else /* no client */
        {
            return NX_NULL;
        }
    }

	/* update channel source */
    channel->sourceThread = source;

	return channel;
}

NX_PRIVATE NX_Error PutFreeChannel(NX_Hub *hub, NX_HubChannel *channel)
{
	NX_ASSERT(hub);
	NX_ASSERT(channel);

	NX_UArch level;
	NX_SpinLockIRQ(&hub->lock, &level);
	channel->state = NX_HUB_CHANNEL_IDLE;
    channel->sourceThread = NX_NULL;
    channel->targetThread = NX_NULL;
	NX_SpinUnlockIRQ(&hub->lock, level);
	
	return NX_EOK;
}

NX_Error NX_HubRegister(const char *name, NX_Size maxClient, NX_Hub **outHub)
{
	NX_Hub *hub;
	NX_UArch level;
    NX_Thread *self;

	if (!name)
	{
		return NX_EINVAL;
	}

    if (!maxClient)
    {
        maxClient = NX_HUB_CLIENTS_MAX;
    }

	if (SearchHub(name) != NX_NULL) /* hub has exist! */
	{
		return NX_EBUSY;
	}

	hub = CreateHub(name, maxClient);
	if (hub == NX_NULL)
	{
		return NX_ENOMEM;
	}

	NX_SpinLockIRQ(&hubSystemLock, &level);
	NX_ListAdd(&hub->list, &hubSystemListHead);
	NX_SpinUnlockIRQ(&hubSystemLock, level);

    self = NX_ThreadSelf();
	self->resource.hub = hub;

	if (outHub)
	{
		*outHub = hub;
	}

	return NX_EOK;
}

NX_Error NX_HubUnregister(const char *name)
{
	NX_Hub *hub;
	NX_UArch level;
    NX_Thread *self;

	if (!name)
	{
		return NX_EINVAL;
	}

	hub = SearchHub(name);
	if (!hub)
	{
		return NX_ENOSRCH;
	}

    self = NX_ThreadSelf();

    if (self->resource.hub != hub)
    {
        return NX_ENORES;
    }

    if (NX_AtomicGet(&hub->totalChannels) > 0)
    {
        return NX_EBUSY;
    }

	self->resource.hub = NX_NULL;

	NX_SpinLockIRQ(&hubSystemLock, &level);
	NX_ListDel(&hub->list);
	NX_SpinUnlockIRQ(&hubSystemLock, level);
	
	DestroyHub(hub);

	return NX_EOK;
}

NX_Error HubMigrateToServer(NX_Hub *hub, NX_HubParam *param, NX_HubChannel *channel)
{
	NX_UArch level;

	NX_CopyFromUserEx(&channel->param, param);
	
	NX_SpinLockIRQ(&hub->lock, &level);
	channel->state = NX_HUB_CHANNEL_ACTIVE;
	NX_SpinUnlockIRQ(&hub->lock, level);

    NX_SemaphoreWait(&channel->syncSem); /* wait server poll channel */
	return NX_EOK;
}

NX_Error NX_HubCallParam(NX_Hub *hub, NX_HubParam *param, NX_Size *retVal)
{
	NX_HubChannel *channel;
	NX_Thread *cur;
	NX_Error retErr;
	
	if (!hub || !param)
	{
		return NX_EINVAL;
	}
	
	cur = NX_ThreadSelf();
	/* get a free channel */
	channel = GetFreeChannel(hub, cur);
	if (channel == NX_NULL)
	{
		return NX_EAGAIN;
	}

	HubMigrateToServer(hub, param, channel);

    if (retVal)
    {
    	NX_CopyToUser((void *)retVal, (void *)&channel->retVal, sizeof(channel->retVal));
    }

	retErr = channel->retErr;

	/* put channel after used */
    PutFreeChannel(hub, channel);

	return retErr;
}

NX_Error NX_HubCallParamName(const char *name, NX_HubParam *param, NX_Size *retVal)
{
    NX_Hub *hub = SearchHub(name);
    if (!hub)
    {
        return NX_ENOSRCH;
    }
    return NX_HubCallParam(hub, param, retVal);
}

NX_PRIVATE NX_Error HubMigrateToClient(NX_Hub *hub, NX_HubChannel *channel, NX_Size retVal, NX_Error retErr)
{
	channel->retVal = retVal;
	channel->retErr = retErr;

    /* channel handled */
    NX_SemaphoreSignal(&channel->syncSem);
	return NX_EOK;
}

NX_Error NX_HubReturn(NX_Size retVal, NX_Error retErr)
{
	NX_Thread *cur = NX_ThreadSelf();
	NX_Hub *hub = cur->resource.hub;
	NX_HubChannel *channel = cur->resource.activeChannel;
	
	if (!hub)
	{
		return NX_EFAULT;
	}

	if (!channel)
	{
		return NX_EAGAIN;
	}

	if (channel->targetThread != cur)
	{
		NX_LOG_E("hub return: channel targe thread error!");
		return NX_EFAULT;
	}

    /* clear active channel */
	cur->resource.activeChannel = NX_NULL;

	/* release mdl */
	NX_HubReleaseMdl(channel);

	return HubMigrateToClient(hub, channel, retVal, retErr);
}

NX_PRIVATE void HubCheckChannelRelease(NX_Hub *hub)
{
    NX_ASSERT(hub);

    if (NX_AtomicGet(&hub->totalChannels) > 0)
    {    
        if (!hub->idleTime)
        {
            hub->idleTime = NX_ClockTickGetMillisecond();
        }

        /* check idle time, release all channels */
        if (NX_ClockTickGetMillisecond() - hub->idleTime > NX_HUB_IDLE_TIME * 1000)
        {
            NX_HubChannel *channel, *nextChannel;
            NX_UArch level;

            NX_SpinLockIRQ(&hub->lock, &level);
            NX_ListForEachEntrySafe(channel, nextChannel, &hub->channelListHead, list)
            {
                HubDelChannelLocked(hub, channel);
                DestroyChannel(channel);
            }
        	NX_SpinUnlockIRQ(&hub->lock, level);
            hub->idleTime = 0; /* clear idle time */
        }
    }
}

NX_Error NX_HubPoll(NX_HubParam *param)
{
	NX_UArch level;
	NX_HubChannel *channel, *channelActive = NX_NULL;
	NX_Thread *thread = NX_ThreadSelf();
	NX_Hub *hub = thread->resource.hub;
	NX_Error err = NX_EAGAIN;

	if (!hub) /* thread no hub resource */
	{
		return NX_ENORES;
	}

	/* poll channels on hub */
	NX_SpinLockIRQ(&hub->lock, &level);

	NX_ListForEachEntry(channel, &hub->channelListHead, list)
	{
        /* channel active and source thread in  */
		if (channel->state == NX_HUB_CHANNEL_ACTIVE)
		{
            channel->state = NX_HUB_CHANNEL_HANDLED;
            channelActive = channel;
			break;
		}
	}
	NX_SpinUnlockIRQ(&hub->lock, level);

    if (channelActive) /* handle active channel */
    {
        hub->idleTime = 0; /* clear idle time */

        NX_CopyToUserEx(param, &channelActive->param);
        channelActive->targetThread = thread;

        thread->resource.activeChannel = channelActive;
        err = NX_EOK;
    }
    else
    {
        HubCheckChannelRelease(hub);
    }

	return err;
}

NX_PRIVATE NX_HubMdl *CreateMdl(NX_HubChannel *channel, NX_Addr addr, NX_Size len, NX_Vmspace *space)
{
    NX_HubMdl *mdl = NX_MemAllocEx(NX_HubMdl);
    if (mdl == NX_NULL)
    {
        return NX_NULL;
    }
    NX_ListInit(&mdl->list);
    mdl->startAddr = addr & NX_PAGE_ADDR_MASK;
    mdl->mappedAddr = NX_NULL; 
    mdl->byteOffset = addr - mdl->startAddr;
    if (len > NX_HUB_MDL_LEN_MAX)
    {
        len = NX_HUB_MDL_LEN_MAX;
    }
    mdl->byteCount = len;
    mdl->vmspace = space;

    NX_ListAdd(&mdl->list, &channel->mdlListHead);

    return mdl;
}

NX_PRIVATE void DestroyMdl(NX_HubMdl *mdl)
{
	NX_ASSERT(mdl);

    NX_ListDelInit(&mdl->list);
    NX_MemFree(mdl);
}

NX_PRIVATE void *MapMdl(NX_HubMdl *mdl, NX_Vmspace *sourceSpace, NX_Vmspace *targetSpace)
{
    NX_Addr phyAddr;
	NX_Addr virAddr;
	void *mapAddr = NX_NULL;
	
	NX_Size pageCount;
	NX_Size pageIdx;
	NX_Size pageCountMapped;
	
	NX_ASSERT(mdl);
	NX_ASSERT(sourceSpace);
	NX_ASSERT(targetSpace);

	pageCount = NX_DIV_ROUND_UP(mdl->byteOffset + mdl->byteCount, NX_PAGE_SIZE);
	
	/* check addr valid */
	for (pageIdx = 0, virAddr = mdl->startAddr; pageIdx < pageCount; pageIdx++, virAddr += NX_PAGE_SIZE)
	{
		phyAddr = NX_VmspaceVirToPhy(sourceSpace, (NX_Addr)virAddr);
		if (phyAddr == 0)
		{
			NX_LOG_E("hub map: addr %p not in process!", virAddr);
			return NX_NULL;
		}
	}

	/* map memory */
	/* FIXME: lock vmspace while map virtual addr, Make sure the addresses are consecutive */
	pageCountMapped = 0;
	for (pageIdx = 0, virAddr = mdl->startAddr; pageIdx < pageCount; pageIdx++, virAddr += NX_PAGE_SIZE)
	{
		phyAddr = NX_VmspaceVirToPhy(sourceSpace, (NX_Addr)virAddr);
		NX_ASSERT(phyAddr);

		if (NX_VmspaceMapWithPhy(targetSpace, 0, phyAddr, NX_PAGE_SIZE, NX_PAGE_ATTR_USER, 0, &mapAddr) != NX_EOK)
		{
			if (pageCountMapped > 0) /* unmap mapped pages */
			{
				NX_VmspaceUnmap(targetSpace, mdl->startAddr, pageCountMapped * NX_PAGE_SIZE);
			}
			return NX_NULL;
		}

		pageCountMapped++;

		if (mdl->mappedAddr == NX_NULL)
		{
			mdl->mappedAddr = mapAddr;
		}
	}
	NX_ASSERT(mdl->mappedAddr);
	return (void *)((NX_U8 *)mdl->mappedAddr + mdl->byteOffset);
}

NX_PRIVATE void UnmapMdl(NX_HubMdl *mdl)
{
	NX_Addr virAddr;
	NX_Size pageCount;
	NX_Size pageIdx;
	NX_Vmspace *vmspace = mdl->vmspace;

	NX_ASSERT(mdl);
	NX_ASSERT(vmspace);

	pageCount = NX_DIV_ROUND_UP(mdl->byteOffset + mdl->byteCount, NX_PAGE_SIZE);
	
	/* FIXME: lock vmspace while unmap virtual addr, Make sure the addresses are consecutive */
	virAddr = (NX_Addr)mdl->mappedAddr;
	for (pageIdx = 0; pageIdx < pageCount; pageIdx++, virAddr += NX_PAGE_SIZE)
	{
		if (NX_VmspaceUnmap(vmspace, virAddr, NX_PAGE_SIZE) != NX_EOK)
		{
			NX_LOG_W("hub mdl: unmap vitural addr %p error", virAddr);
		}
	}
}

NX_PRIVATE void *NX_HubMapAddr(NX_HubChannel *channel, NX_Thread *source, NX_Thread *target, void *addr, NX_Size size)
{
    NX_Process *sourceProc, *targetProc;
    void *mapAddr;
    NX_HubMdl *mdl;

    targetProc = target->resource.process;
    sourceProc = source->resource.process;
    if (targetProc == NX_NULL || sourceProc == NX_NULL)
    {
		NX_LOG_E("hub map: thread not in process!");
        return NX_NULL;
    }

	/* limit mdl list length */
	if (NX_ListLength(&channel->mdlListHead) >= NX_HUB_PARAM_NR)
	{
		NX_LOG_E("hub mdl: too much mdl! max mdl is %d .", NX_HUB_PARAM_NR);
		return NX_NULL;
	}
	
    mdl = CreateMdl(channel, (NX_Addr)addr, size, &targetProc->vmspace);
    if (mdl == NX_NULL)
    {
        NX_LOG_E("hub map: create mdl no enough memory!");
        return NX_NULL;
    }

	mapAddr = MapMdl(mdl, &sourceProc->vmspace, &targetProc->vmspace);
	if (mapAddr == NX_NULL)
	{
        NX_LOG_E("hub map: map mdl failed!");
        DestroyMdl(mdl);
		return NX_NULL;
	}
    return (char *)mapAddr;
}

NX_PRIVATE NX_Bool ChannelMatchParam(NX_Size arg, NX_HubChannel *channel)
{
	int i;
	for (i = 0; i < NX_HUB_PARAM_NR; i++)
	{
		if (arg == channel->param.args[i])
		{
			return NX_True;
		}
	}
	return NX_False;
}

void *NX_HubTranslate(void *addr, NX_Size size)
{
    NX_Thread *source, *target;

    NX_Thread *cur = NX_ThreadSelf();
	NX_Hub *hub = cur->resource.hub;
	NX_HubChannel *channel = cur->resource.activeChannel;

    if (!addr || !size)
    {
        return NX_NULL;
    }

	if (!hub)
	{
		return NX_NULL;
	}

	if (!channel)
	{
		return NX_NULL;
	}

    target = channel->targetThread;

	if (target != cur)
	{
		NX_LOG_E("hub translate: channel targe thread error!");
		return NX_NULL;
	}

    source = channel->sourceThread;

    if (source == NX_NULL)
	{
		NX_LOG_E("hub translate: channel source thread error!");
		return NX_NULL;
	}

	/* check addr in channel param */
	if (ChannelMatchParam((NX_Size)addr, channel) == NX_False)
	{
		NX_LOG_E("hub translate: arg %p not in channel param!", addr);
		return NX_NULL;
	}
	
    return NX_HubMapAddr(channel, source, target, addr, size);
}

NX_PRIVATE void NX_HubReleaseMdl(NX_HubChannel *channel)
{
	NX_HubMdl *mdl, *next;

	NX_ListForEachEntrySafe(mdl, next, &channel->mdlListHead, list)
	{
		UnmapMdl(mdl);
		DestroyMdl(mdl);
	}
}
