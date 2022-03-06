/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: IRQ
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-28     JasonHu           Init
 */

#include <io/irq.h>
#include <io/delay_irq.h>
#include <utils/memory.h>
#include <utils/string.h>
#include <mm/alloc.h>

NX_PRIVATE NX_IRQ_Node IRQ_NodeTable[NX_NR_IRQS];

void NX_IRQ_Init(void)
{
    int i;
    NX_IRQ_Node *irq;
    for (i = 0; i < NX_NR_IRQS; i++)
    {
        irq = &IRQ_NodeTable[i];
        irq->flags = 0;
        irq->controller = NX_NULL;
        NX_AtomicSet(&irq->reference, 0);
        NX_ListInit(&irq->actionList);
    }
    NX_IRQ_DelayQueueInit();
}

NX_PRIVATE NX_IRQ_Node *IRQ_NodeGet(NX_IRQ_Number irq)
{
    if (0 <= irq && irq < NX_NR_IRQS) {
        return &IRQ_NodeTable[irq];
    }
    return NX_NULL;
}

NX_Error NX_IRQ_Bind(NX_IRQ_Number irqno,
                         NX_IRQ_Handler handler,
                         void *data,
                         char *name,
                         NX_U32 flags)
{
    NX_IRQ_Node *irqNode = IRQ_NodeGet(irqno);
    if (irqNode == NX_NULL)
    {
        return NX_EINVAL;
    }
    irqNode->controller = &NX_IRQ_ControllerInterface;
    irqNode->flags = flags;

    NX_IRQ_Action *action = NX_MemAlloc(sizeof(NX_IRQ_Action));
    if (action == NX_NULL)
    {
        return NX_ENOMEM;
    }
    action->data = data;
    action->flags = flags;
    action->handler = handler;
    NX_MemZero(action->name, NX_IRQ_NAME_LEN);
    NX_StrCopyN(action->name, name, NX_IRQ_NAME_LEN);
    
    /* add to action list */
    NX_ListAddTail(&action->list, &irqNode->actionList);
    
    NX_AtomicInc(&irqNode->reference);
    return NX_EOK;
}

NX_Error NX_IRQ_Unbind(NX_IRQ_Number irqno, void *data)
{
    NX_IRQ_Node *irqNode = IRQ_NodeGet(irqno);
    if (irqNode == NX_NULL)
    {
        return NX_EINVAL;
    }
    if (irqNode->controller == NX_NULL)
    {
        return NX_EFAULT;
    }

    NX_IRQ_Action *action = NX_NULL;
    NX_IRQ_Action *actionFind = NX_NULL;

    NX_UArch level = NX_IRQ_SaveLevel();
    if (irqNode->flags & NX_IRQ_FLAG_SHARED)
    {
        NX_ListForEachEntry(action, &irqNode->actionList, list)
        {
            /* find irq device if data are same */
            if (action->data == data)
            {
                actionFind = action;
                break;
            }
        }
    }
    else
    {
        actionFind = NX_ListFirstEntryOrNULL(&irqNode->actionList, NX_IRQ_Action, list);
    }
    if (actionFind == NX_NULL)
    {
        NX_IRQ_RestoreLevel(level);
        return NX_ENORES;
    }
    /* remove action */
    NX_ListDel(&actionFind->list);
    NX_MemFree(actionFind);

    NX_AtomicDec(&irqNode->reference);
    /* no device on this irq */
    if (NX_AtomicGet(&irqNode->reference) == 0)
    {
        irqNode->controller = NX_NULL;
        irqNode->flags = 0;
    }
    NX_IRQ_RestoreLevel(level);
    return NX_EOK;
}

NX_Error NX_IRQ_Unmask(NX_IRQ_Number irqno)
{
    NX_IRQ_Node *irqNode = IRQ_NodeGet(irqno);
    if (irqNode == NX_NULL)
    {
        return NX_EINVAL;
    }
    if (irqNode->controller != NX_NULL && irqNode->controller->unmask != NX_NULL)
    {
        return irqNode->controller->unmask(irqno);
    }
    return NX_ENOFUNC;
}

NX_Error NX_IRQ_Mask(NX_IRQ_Number irqno)
{
    NX_IRQ_Node *irqNode = IRQ_NodeGet(irqno);
    if (irqNode == NX_NULL)
    {
        return NX_EINVAL;
    }
    if (irqNode->controller != NX_NULL && irqNode->controller->mask != NX_NULL)
    {
        return irqNode->controller->mask(irqno);
    }
    return NX_ENOFUNC;
}

NX_Error NX_IRQ_Handle(NX_IRQ_Number irqno)
{
    NX_IRQ_Node *irqNode = IRQ_NodeGet(irqno);
    if (irqNode == NX_NULL)
    {
        return NX_EINVAL;
    }
    NX_IRQ_Action *action;

    /* invoke each action on irq node */
    NX_ListForEachEntry(action, &irqNode->actionList, list)
    {
        if (action->handler(irqno, action->data) == NX_EOK)
        {
            break;
        }
    }
    /* ack irq after handled */
    if (irqNode->controller->ack != NX_NULL)
    {
        irqNode->controller->ack(irqno);
    }
    return NX_EOK;
}
