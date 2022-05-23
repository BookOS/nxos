/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: IO Interrupt ReQuest Delay Queue 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-29     JasonHu           Init
 */

#ifndef __IO_DELAY_IRQ__
#define __IO_DELAY_IRQ__

#include <nxos.h>
#include <base/list.h>
#include <base/irq.h>

enum NX_IRQ_DelayQueue 
{
    NX_IRQ_FAST_QUEUE = 0,
    NX_IRQ_NORMAL_QUEUE,
    NX_IRQ_PERIOD_QUEUE,
    NX_IRQ_SCHED_QUEUE,
    NX_IRQ_SLOW_QUEUE,
    NX_IRQ_QUEUE_NR,
};
typedef enum NX_IRQ_DelayQueue NX_IRQ_DelayQueue;

#define NX_IRQ_WORK_NOREENTER  0x01    /* no reenter when do work */

#define NX_IRQ_DELAY_WORK_CHECK_TIMES 5

typedef void (*NX_IRQ_WorkHandler)(void *arg);

struct NX_IRQ_DelayWork
{
    NX_List list;
    NX_U32 flags;
    NX_IRQ_WorkHandler handler;
    void *arg;
    NX_IRQ_DelayQueue queue;
};
typedef struct NX_IRQ_DelayWork NX_IRQ_DelayWork;

void NX_IRQ_DelayQueueInit(void);

NX_Error NX_IRQ_DelayQueueEnter(NX_IRQ_DelayQueue queue, NX_IRQ_DelayWork *work);
NX_Error NX_IRQ_DelayQueueLeave(NX_IRQ_DelayQueue queue, NX_IRQ_DelayWork *work);

NX_IRQ_DelayWork *NX_IRQ_DelayWorkCreate(NX_IRQ_WorkHandler handler, void *arg, NX_U32 flags);
NX_Error NX_IRQ_DelayWorkDestroy(NX_IRQ_DelayWork *work);
NX_Error NX_IRQ_DelayWorkInit(NX_IRQ_DelayWork *work, NX_IRQ_WorkHandler handler, void *arg, NX_U32 flags);

NX_Error NX_IRQ_DelayWorkHandle(NX_IRQ_DelayWork *work);

NX_INTERFACE void NX_IRQ_DelayQueueCheck(void);

#endif  /* __IO_DELAY_IRQ__ */
