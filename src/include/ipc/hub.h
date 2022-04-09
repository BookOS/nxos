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

#include <xbook.h>
#include <xbook/atomic.h>
#include <sched/spin.h>
#include <sched/mutex.h>
#include <sched/thread.h>
#include <sched/semaphore.h>
#include <process/process.h>
#include <utils/list.h>

#define NX_HUB_SERVER_STACK_SIZE    (16 * NX_KB)

#define NX_HUB_NAME_LEN 32
typedef struct NX_Hub
{
    NX_List list;
    NX_List channelListHead;    /* channel on this hub */
    char name[NX_HUB_NAME_LEN];
    NX_Addr callAddr;
    NX_Size maxClient;
    NX_Process *process;
    NX_Atomic totalChannels;
    NX_Atomic requestClients;
    NX_Spin lock;
} NX_Hub;

#define NX_HUB_PARAM_NR 7
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
    NX_HubChannelState state;
    NX_Thread *sourceThread;
    NX_Thread *targetThread;
    NX_Size retVal;

    NX_Addr serverStackTop;
    NX_Size serverStackSize;
    NX_Hub *hub;
    NX_Semaphore syncSem;
    NX_HubParam param;
} NX_HubChannel;

NX_Error NX_HubRegister(const char *name, NX_Addr callAddr, NX_Size maxClient, NX_Hub **outHub);
NX_Error NX_HubUnregister(const char *name);

NX_Error NX_HubCallParam(NX_Hub *hub, NX_HubParam *param, NX_Size *retVal);
NX_Error NX_HubCallParamName(const char *name, NX_HubParam *param, NX_Size *retVal);

NX_Error NX_HubPoll(NX_HubParam *param);
NX_Error NX_HubReturn(NX_Size retVal);

void NX_HubDump(NX_Hub *hub);
void NX_HubChannelDump(NX_HubChannel *channel);

#endif /* __IPC_HUB_H__ */
