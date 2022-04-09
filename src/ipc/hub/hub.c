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

#include <xbook.h>
#include <ipc/hub.h>
#include <mm/alloc.h>
#include <mm/page.h>
#include <utils/string.h>
#include <xbook/debug.h>
#include <utils/log.h>
#include <utils/memory.h>
#include <mm/vmspace.h>
#include <process/uaccess.h>

/*
ret = HubCall(hub, HubCallNr, args);

HubCall: 
	hub -> thread

	build hubcall param

	migrate to server(thread, )


*/

/*
hubcall(hubcallParam):
-> get hub call nr
	-> get call handler(callNr)
	-> handler(arg0, arg1, ...)

-> call ret or auto call ret after call done().
*/

/*
call loop
yield to other thread/
wait call come
*/

/*
signal

setsignal

-> sig to user
	-> return to user
	-> build signal
	-> return to user stack
	-> call handler
	-> call ret
*/

NX_PRIVATE NX_LIST_HEAD(HubSystemListHead);
NX_PRIVATE NX_SPIN_DEFINE_UNLOCKED(HubSystemLock);
 
void NX_HubDump(NX_Hub *hub)
{
    NX_LOG_D("------------ Dump Hub ------------");
    NX_LOG_D("hub %p, name: %s, call addr: %p, max client: %d", hub, hub->name, hub->callAddr, hub->maxClient);
    NX_LOG_D("hub process: %p, total channels: %d, request clients: %d", 
        hub->process, NX_AtomicGet(&hub->totalChannels), NX_AtomicGet(&hub->requestClients));    
    NX_LOG_D("====================================================");
}

void NX_HubChannelDump(NX_HubChannel *channel)
{
    NX_LOG_D("------------ Dump Hub Channel ----------------");
    NX_LOG_D("hub cahnnel source: %p/%s, target: %p/%s, retval: %p",
        channel->sourceThread, channel->sourceThread ? channel->sourceThread->name : "null",
        channel->targetThread, channel->targetThread ? channel->targetThread->name : "null");
    NX_LOG_D("hub cahnnel server stack top: %p, server stack size: %p, state: %d", 
        channel->serverStackTop, channel->serverStackSize, channel->state);    
    NX_LOG_D("====================================================");
}

NX_PRIVATE void HubChannelInit(NX_HubChannel *channel,
    NX_Thread *source,
    NX_Thread *target,
    NX_Addr serverStackTop,
    NX_Size serverStackSize,
    NX_Hub *hub)
{
	channel->sourceThread = source;
	channel->targetThread = target;
	channel->state = NX_HUB_CHANNEL_IDLE;
	NX_ListInit(&channel->list);
	channel->retVal = 0;
	channel->serverStackSize = serverStackSize;
	channel->serverStackTop = serverStackTop;
    channel->hub = hub;
    NX_SemaphoreInit(&channel->syncSem, 0);
}

NX_PRIVATE NX_HubChannel *CreateChannel(NX_Hub *hub, NX_Thread *source, NX_Thread *target)
{
    NX_Error err;
    void *addr = NX_NULL;
	NX_HubChannel *channel = NX_MemAllocEx(NX_HubChannel);
	if (channel == NX_NULL)
	{
		return NX_NULL;
	}
	
    /* TODO: create server stack */
    err = NX_VmspaceMap(&hub->process->vmspace, 0, NX_HUB_SERVER_STACK_SIZE, NX_PAGE_ATTR_USER, 0, &addr);
    if (err != NX_EOK)
    {
        NX_MemFree(channel);
        return NX_NULL;
    }

    HubChannelInit(channel, source, target, (NX_Addr)addr + NX_HUB_SERVER_STACK_SIZE, NX_HUB_SERVER_STACK_SIZE, hub);

	return channel;
}

NX_PRIVATE NX_Error DestroyChannel(NX_HubChannel *channel)
{
    NX_Error err;
    NX_Hub *hub;
	NX_ASSERT(channel);

    hub = channel->hub;
    if (hub == NX_NULL)
    {
        return NX_EFAULT;
    }
    /* destroy stack */
    err = NX_VmspaceUnmap(&hub->process->vmspace, channel->serverStackTop - channel->serverStackSize,
        channel->serverStackSize);

    if (err != NX_EOK)
    {
        NX_LOG_E("hub channel unmap stack base %p, size %p error!");
        return NX_EFAULT;
    }

	NX_MemFree(channel);
	return err;
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

NX_PRIVATE NX_Error HubDelChannel(NX_Hub *hub, NX_HubChannel *channel)
{
	NX_UArch level;
	NX_ASSERT(hub);
	NX_ASSERT(channel);
	
	NX_SpinLockIRQ(&hub->lock, &level);
	NX_ListDel(&channel->list);
	NX_AtomicDec(&hub->totalChannels);
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

NX_PRIVATE void HubInit(NX_Hub *hub, const char *name, NX_Addr callAddr, NX_Size maxClient)
{
	NX_StrCopy(hub->name, name);
	
	hub->callAddr = callAddr;
	hub->maxClient = maxClient;

	hub->process = NX_NULL;
	NX_AtomicSet(&hub->totalChannels, 0);
	NX_AtomicSet(&hub->requestClients, 0);
	NX_ListInit(&hub->channelListHead);
	NX_ListInit(&hub->list);
	
	NX_SpinInit(&hub->lock);
}

NX_PRIVATE NX_Error DestroyHub(NX_Hub *hub)
{
	NX_MemFree(hub);
	return NX_EOK;
}

NX_PRIVATE NX_Hub *CreateHub(const char *name, NX_Addr callAddr, NX_Size maxClient)
{
    NX_Hub *hub;
    NX_Thread *cur;
    NX_Process *process;
    NX_HubChannel *channel;

	cur = NX_ThreadSelf();

    process = cur->resource.process;
    if (process == NX_NULL) /* only process can create hub */
    {
        return NX_NULL;
    }

	hub = NX_MemAllocEx(NX_Hub);
	if (hub == NX_NULL)
	{
		return NX_NULL;
	}
	
    HubInit(hub, name, callAddr, maxClient);

    hub->process = process; /* bind server thread */

    /* create hub channel */
    channel = CreateChannel(hub, cur, cur);
    if (channel == NX_NULL)
    {
        DestroyHub(hub);
        return NX_NULL;
    }

    HubAddChannel(hub, channel);

	return hub;
}

NX_PRIVATE NX_Hub *SearchHub(const char *name)
{
	NX_Hub *hub;
	NX_ListForEachEntry(hub, &HubSystemListHead, list)
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
    // FIXME: reset state
	NX_SpinUnlockIRQ(&hub->lock, level);
	
	return NX_EOK;
}

NX_Error NX_HubRegister(const char *name, NX_Addr callAddr, NX_Size maxClient, NX_Hub **outHub)
{
	NX_Hub *hub;
	NX_UArch level;

	if (!name || !callAddr || !maxClient)
	{
		return NX_EINVAL;
	}

	hub = CreateHub(name, callAddr, maxClient);
	if (hub == NX_NULL)
	{
		return NX_ENOMEM;
	}

	NX_SpinLockIRQ(&HubSystemLock, &level);
	NX_ListAdd(&hub->list, &HubSystemListHead);
	NX_SpinUnlockIRQ(&HubSystemLock, level);

	NX_ThreadSelf()->resource.hub = hub;

	if (outHub)
	{
		*outHub = hub;
	}

    NX_HubDump(hub);

	return NX_EOK;
}

NX_Error NX_HubUnregister(const char *name)
{
	NX_Hub *hub;
	NX_UArch level;

	if (!name)
	{
		return NX_EINVAL;
	}

	hub = SearchHub(name);
	if (!hub)
	{
		return NX_ENOSRCH;
	}

	/* FIXME: check hub not active */
	
	NX_SpinLockIRQ(&HubSystemLock, &level);
	NX_ListDel(&hub->list);
	NX_SpinUnlockIRQ(&HubSystemLock, level);
	
	DestroyHub(hub);

	return NX_EOK;
}

NX_Error HubBuildCallEnv(NX_Hub *hub, NX_HubParam *param, NX_HubChannel *channel)
{
	NX_U8 *top;
    NX_U8 *paramAddr;
    /* pass arg as reg */
    top = (NX_U8 *)NX_VmspaceVirToPhy(&hub->process->vmspace, channel->serverStackTop - channel->serverStackSize);
    if (top == NX_NULL)
    {
        return NX_EFAULT;
    }
    top = NX_Phy2Virt(top);

    top -= sizeof(NX_HubParam);
    top = (NX_U8 *)NX_ALIGN_DOWN(((NX_Addr)top), 4);; /* 4 byte align */
    paramAddr = top;
    /* copy param */
    NX_CopyFromUser((char *)top, (char *)param, sizeof(param));
    
    /* copy param addr */
    --top;
    NX_MemCopy(top, &paramAddr, sizeof(NX_U32));
    
    /* TODO: build a frame */

	return NX_EOK;
}

NX_Error HubMigrateToServer(NX_Hub *hub, NX_HubParam *param, NX_HubChannel *channel)
{
	NX_UArch level;

	NX_CopyFromUserEx(&channel->param, param);
	
	NX_SpinLockIRQ(&hub->lock, &level);
	channel->state = NX_HUB_CHANNEL_ACTIVE;
	NX_SpinUnlockIRQ(&hub->lock, level);

    // NX_HubChannelDump(channel);

    // NX_LOG_D("migrate to server: %d", NX_AtomicGet(&channel->syncSem.value));

    /* TODO: sched to server, block self */
	// NX_ThreadSleep(1000);

	// WaitSemaphore();
    // mutex lock.
    NX_SemaphoreWait(&channel->syncSem);
    // NX_SemaphoreWait(&channel->syncSem);

    NX_LOG_D("client wait done: %d", NX_AtomicGet(&channel->syncSem.value));

	return NX_EOK;
}

NX_Error NX_HubCallParam(NX_Hub *hub, NX_HubParam *param, NX_Size *retVal)
{
	NX_HubChannel *channel;
	NX_Thread *cur;
	
	if (!hub || !param || !retVal)
	{
		return NX_EINVAL;
	}
	
    NX_HubDump(hub);

	cur = NX_ThreadSelf();
	/* get a free channel */
	channel = GetFreeChannel(hub, cur);
	if (channel == NX_NULL)
	{
		return NX_EAGAIN;
	}

    NX_HubChannelDump(channel);

	HubMigrateToServer(hub, param, channel);

	*retVal = channel->retVal;

	/* put channel after used */
    PutFreeChannel(hub, channel);

	return NX_EOK;
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

NX_Error HubMigrateToClient(NX_Hub *hub, NX_HubChannel *channel, NX_Size retVal)
{
	channel->retVal = retVal;
	/* TODO: sched to client */
	// SignalSemaphore();
    NX_LOG_D("migrate to client: %d", NX_AtomicGet(&channel->syncSem.value));

    NX_SemaphoreSignal(&channel->syncSem);

	return NX_EOK;
}

NX_Error NX_HubReturn(NX_Size retVal)
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
		NX_LOG_E("channel targe error!");
		return NX_EFAULT;
	}

	cur->resource.activeChannel = NX_NULL;
	return HubMigrateToClient(hub, channel, retVal);
}

NX_Error NX_HubPoll(NX_HubParam *param)
{
	NX_UArch level;
	NX_HubChannel *channel;
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
        /* channel active and source */
		if (channel->state == NX_HUB_CHANNEL_ACTIVE && channel->sourceThread->state == NX_THREAD_BLOCKED)
		{
			NX_LOG_D("get a active channel %p", channel);
			channel->state = NX_HUB_CHANNEL_HANDLED;
			thread->resource.activeChannel = channel;
			err = NX_EOK;
			break;
		}
	}

	NX_SpinUnlockIRQ(&hub->lock, level);
	return err;
}
