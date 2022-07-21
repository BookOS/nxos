/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: thread signal
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-05-28     JasonHu           Init
 */

#ifndef __IPC_SIGNAL_H__
#define __IPC_SIGNAL_H__

#include <nxos.h>
#include <base/list.h>
#include <base/atomic.h>
#include <base/spin.h>

#define NX_SIGNAL_INVALID       0 /* invalid signal */
#define NX_SIGNAL_KILL          1 /* force kill thread (can not block) */
#define NX_SIGNAL_STOP          2 /* stop thread running (can not block) */
#define NX_SIGNAL_INTERRUPT     3 /* interrupt thread running */
#define NX_SIGNAL_IRQ           4 /* hardware irq */
#define NX_SIGNAL_MEMACC        5 /* memory access */
#define NX_SIGNAL_FPE           6 /* float processor exception */
#define NX_SIGNAL_STACKFLOW     7 /* thread stack overflow */
#define NX_SIGNAL_TIMER         8 /* timeout signal */
#define NX_SIGNAL_TRAP          9 /* trap for debug */
#define NX_SIGNAL_EAPI          10 /* exception on call kernel api */
#define NX_SIGNAL_ABORT         11 /* term current process */
#define NX_SIGNAL_CONTINUE      12 /* thread continue run */
#define NX_SIGNAL_TERMINATE     13 /* terminate thread */
#define NX_SIGNAL_SEGV          14 /* segment fault */
#define NX_SIGNAL_ILLEGAL       15 /* illegal instruction */
#define NX_SIGNAL_INST          16 /* instruction aceesss */

/* ...-31 RESERVED */
#define NX_SIGNAL_ANONYMOUS     32 /* anonymous signal */
#define NX_SIGNAL_MAX           64 /* max signal */

#define NX_SIGNAL_CMD_BLOCK     1 /* block signal */
#define NX_SIGNAL_CMD_UNBLOCK   2 /* unblock signal */

#define NX_SIGNAL_HANDLER_DEFAULT   ((void *)0) /* default handler means exit thread */
#define NX_SIGNAL_HANDLER_IGNORE    ((void *)1) /* ignore the signal, expect KILL and STOP */

#define NX_SIGNAL_FLAG_RESTART      0x01    /* restart things after handle signal */

typedef NX_U32 NX_Signal;

typedef void (*NX_SignalTimerHandler)(void * arg);

typedef struct NX_SignalExtralInfo
{
    NX_SignalTimerHandler timerHandler;
    void * timerArg;
} NX_SignalExtralInfo;

typedef struct NX_SignalInfo
{
    NX_Signal signal;
    void * signalValue;
    NX_U32 tid; /* who send the signal */
    NX_SignalExtralInfo extralInfo;
} NX_SignalInfo;

typedef struct NX_SignalInfoEntry
{
    NX_List list;
    NX_SignalInfo info;
} NX_SignalInfoEntry;

typedef void (*NX_SignalHandler)(NX_SignalInfo * info);

typedef struct NX_SignalAttr
{
    NX_SignalHandler handler;
    NX_U32 flags;
} NX_SignalAttr;

typedef struct NX_SignalEntry
{
    NX_SignalAttr attr; /* signal attr */
    NX_List signalInfoListHead;
    NX_Atomic pendingSignals;
    NX_Spin lock;
    NX_Bool blocked;
} NX_SignalEntry;

typedef struct NX_SignalTable
{
    NX_SignalEntry signalEntry[NX_SIGNAL_MAX];
    NX_Atomic globalPendingSignals; /* has signal pending */
    NX_Atomic userHandled; /* user handled */
} NX_SignalTable;

NX_Error NX_SignalTableInit(NX_SignalTable * table);
NX_Error NX_SignalTableExit(NX_SignalTable * table);

NX_Error NX_SignalGetAttr(NX_Signal signal, NX_SignalAttr * outAttr);
NX_Error NX_SignalSetAttr(NX_Signal signal, NX_SignalAttr * attr);

NX_Error NX_SignalContorl(NX_Signal signalFirst, NX_Signal signalLast, NX_U32 cmd);

NX_Error NX_SignalSend(NX_U32 tid, NX_Signal signal, void * signalValue, NX_SignalExtralInfo * extralInfo);

NX_Size NX_SignalCurrentPending(void);

void NX_SignalCheck(void * trapframe);
void NX_SignalExitHandleUser(void);

#endif /* __IPC_SIGNAL_H__ */
