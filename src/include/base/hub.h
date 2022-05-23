/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Hub system
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-6       JasonHu           Init
 */

#ifndef __IPC_HUB_H__
#define __IPC_HUB_H__

#include <nxos.h>
#include <base/atomic.h>
#include <base/spin.h>
#include <base/mutex.h>
#include <base/thread.h>
#include <base/semaphore.h>
#include <base/list.h>
#include <base/clock.h>

#define NX_HUB_IDLE_TIME 10 /* seconds */

#define NX_HUB_NAME_LEN 32
typedef struct NX_Hub
{
    NX_List list;
    NX_List channelListHead;    /* channel on this hub */
    char name[NX_HUB_NAME_LEN];
    NX_Size maxClient;
    NX_Atomic totalChannels;
    NX_Spin lock;
    NX_TimeVal idleTime;
} NX_Hub;

#define NX_HUB_PARAM_NR 8
typedef struct NX_HubParam
{
    NX_U32 api;
    NX_Size args[NX_HUB_PARAM_NR];
} NX_HubParam;

typedef enum NX_HubChannelState
{
    NX_HUB_CHANNEL_IDLE = 0,
    NX_HUB_CHANNEL_READY,
    NX_HUB_CHANNEL_ACTIVE,
    NX_HUB_CHANNEL_HANDLED,
} NX_HubChannelState;

typedef struct NX_HubChannel
{
    NX_List list;
    NX_List mdlListHead;
    NX_HubChannelState state;
    NX_Thread *sourceThread;
    NX_Thread *targetThread;
    NX_Size retVal;
    NX_Error retErr;
    NX_Hub *hub;
    NX_Semaphore syncSem;
    NX_HubParam param;
} NX_HubChannel;

#define NX_HUB_MDL_LEN_MAX (32 * NX_MB)

/* hub memory description list */
typedef struct NX_HubMdl
{
    NX_List list;
    NX_Addr startAddr;  /* start addr (page aligned) */
    void *mappedAddr; /* mapped addr (page aligned) */
    NX_Size byteOffset; /* offset byte in page */
    NX_Size byteCount;  /* byte count in this mdl */
    NX_Vmspace *vmspace; /* mdl map space */
} NX_HubMdl;

NX_Error NX_HubRegister(const char *name, NX_Size maxClient, NX_Hub **outHub);
NX_Error NX_HubUnregister(const char *name);

NX_Error NX_HubCallParam(NX_Hub *hub, NX_HubParam *param, NX_Size *retVal);
NX_Error NX_HubCallParamName(const char *name, NX_HubParam *param, NX_Size *retVal);

NX_Error NX_HubPoll(NX_HubParam *param);
NX_Error NX_HubReturn(NX_Size retVal, NX_Error retErr);

void *NX_HubTranslate(void *addr, NX_Size size);

void NX_HubDump(NX_Hub *hub);
void NX_HubChannelDump(NX_HubChannel *channel);

#endif /* __IPC_HUB_H__ */
