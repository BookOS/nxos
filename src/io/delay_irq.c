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

#include <io/delay_irq.h>
#include <mm/alloc.h>

/* protect flags */
#define NX_IRQ_WORK_PENDING        0x80000000    /* work is pending */
#define NX_IRQ_WORK_ON_QUEUED      0x40000000    /* work is on queue */

NX_PRIVATE NX_List DelayIrqListTable[NX_IRQ_QUEUE_NR];
NX_PRIVATE NX_VOLATILE NX_U32 DelayIrqEvent;

void NX_IRQ_DelayQueueInit(void)
{
    int i;
    for (i = 0; i < NX_IRQ_QUEUE_NR; i++)
    {
        NX_ListInit(&DelayIrqListTable[i]);
    }
    DelayIrqEvent = 0;
}

NX_PRIVATE NX_U32 IRQ_DelayEventGet(void)
{
    return DelayIrqEvent;
}

NX_PRIVATE void IRQ_DelayEventSet(NX_U32 event)
{
    DelayIrqEvent |= event;
}

NX_PRIVATE void IRQ_DelayEventClear(void)
{
    DelayIrqEvent = 0;
}

NX_Error NX_IRQ_DelayQueueEnter(NX_IRQ_DelayQueue queue, NX_IRQ_DelayWork *work)
{
    if (queue < 0 || queue >= NX_IRQ_QUEUE_NR || work == NX_NULL)
    {
        return NX_EINVAL;
    }
    
    if (NX_ListFind(&work->list, &DelayIrqListTable[queue]))
    {
        return NX_EAGAIN;
    }

    NX_UArch level = NX_IRQ_SaveLevel();
    work->queue = queue;
    work->flags |= NX_IRQ_WORK_ON_QUEUED;
    NX_ListAddTail(&work->list, &DelayIrqListTable[queue]);
    NX_IRQ_RestoreLevel(level);
    return NX_EOK;
}

NX_Error NX_IRQ_DelayQueueLeave(NX_IRQ_DelayQueue queue, NX_IRQ_DelayWork *work)
{
    if (queue < 0 || queue >= NX_IRQ_QUEUE_NR || work == NX_NULL)
    {
        return NX_EINVAL;
    }
    
    if (!NX_ListFind(&work->list, &DelayIrqListTable[queue]))
    {
        return NX_ENOSRCH;
    }

    NX_UArch level = NX_IRQ_SaveLevel();
    work->queue = 0;
    work->flags &= ~NX_IRQ_WORK_ON_QUEUED;

    NX_ListDel(&work->list);
    NX_IRQ_RestoreLevel(level);
    return NX_EOK;
}

NX_Error NX_IRQ_DelayWorkInit(NX_IRQ_DelayWork *work, NX_IRQ_WorkHandler handler, void *arg, NX_U32 flags)
{
    if (work == NX_NULL || handler == NX_NULL)
    {
        return NX_EINVAL;
    }
    work->handler = handler;
    work->arg = arg;
    work->flags = flags;
    work->flags &= ~NX_IRQ_WORK_NOREENTER; /*  */
    NX_ListInit(&work->list);
    return NX_EOK;
}

NX_IRQ_DelayWork *NX_IRQ_DelayWorkCreate(NX_IRQ_WorkHandler handler, void *arg, NX_U32 flags)
{
    NX_IRQ_DelayWork *work = NX_MemAlloc(sizeof(NX_IRQ_DelayWork));
    if (work == NX_NULL)
    {
        return NX_NULL;
    }
    if (NX_IRQ_DelayWorkInit(work, handler, arg, flags) != NX_EOK)
    {
        NX_MemFree(work);
        return NX_NULL;
    }
    return work;
}

NX_Error NX_IRQ_DelayWorkDestroy(NX_IRQ_DelayWork *work)
{
    if (work == NX_NULL)
    {
        return NX_EINVAL;
    }
    NX_MemFree(work);
    return NX_EOK;
}

/**
 * Must called with interrupt disabled
 */
NX_Error NX_IRQ_DelayWorkHandle(NX_IRQ_DelayWork *work)
{
    if (work == NX_NULL)
    {
        return NX_EINVAL;
    }
    if (!(work->flags & NX_IRQ_WORK_ON_QUEUED))
    {
        return NX_EFAULT;
    }
    if (work->queue < 0 || work->queue >= NX_IRQ_QUEUE_NR)
    {
        return NX_EFAULT;
    }
    IRQ_DelayEventSet((1 << work->queue));
    work->flags |= NX_IRQ_WORK_PENDING;

    return NX_EOK;
}

NX_INLINE void IRQ_DelayWorkCheck(NX_IRQ_DelayWork *work)
{
    if (work->flags & NX_IRQ_WORK_PENDING)
    {
        work->flags &= ~NX_IRQ_WORK_PENDING; /* clear pending */

        if (!(work->flags & NX_IRQ_WORK_NOREENTER))
        {
            NX_IRQ_Enable();   
        }

        work->handler(work->arg);

        if (!(work->flags & NX_IRQ_WORK_NOREENTER))
        {
            NX_IRQ_Disable();                      
        }
    }
}

/**
 * Must called interrupt disabled
 */
NX_INTERFACE void NX_IRQ_DelayQueueCheck(void)
{
    int checkTimes = NX_IRQ_DELAY_WORK_CHECK_TIMES;
    
    while (checkTimes-- > 0)
    {
        NX_U32 irqEvent = IRQ_DelayEventGet();
        IRQ_DelayEventClear();
        if (irqEvent == 0)
        {
            return;
        }
        NX_IRQ_Enable();

        int i;
        for (i = 0; i < NX_IRQ_QUEUE_NR; i++)
        {
            if (!(irqEvent & (1 << i)))
            {
                continue;
            }

            NX_IRQ_DelayWork *work;
            NX_ListForEachEntry(work, &DelayIrqListTable[i], list)
            {
                IRQ_DelayWorkCheck(work);
            }
        }

        NX_IRQ_Disable();
    }
}
