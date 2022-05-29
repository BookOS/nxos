/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: signal ipc
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-05-29     JasonHu           Init
 */

#include <base/signal.h>
#include <base/thread.h>
#include <base/malloc.h>
#include <base/debug.h>
#include <base/log.h>
#include <base/irq.h>

NX_Error NX_SignalTableInit(NX_SignalTable * table)
{
    int i;
    NX_SignalEntry * entry;

    if (!table)
    {
        return NX_EINVAL;
    }
    
    for (i = 0; i < NX_SIGNAL_MAX; i++)
    {
        entry = &table->signalEntry[i];

        entry->attr.handler = NX_SIGNAL_HANDLER_DEFAULT;
        entry->attr.flags = 0;
        entry->blocked = NX_False;
        NX_ListInit(&entry->signalInfoListHead);
        NX_SpinInit(&entry->lock);
        NX_AtomicSet(&entry->pendingSignals, 0);    
    }
    NX_AtomicSet(&table->globalPendingSignals, 0);
    return NX_EOK;
}

NX_Error NX_SignalInitAttr(NX_SignalAttr * attr, NX_SignalHandler handler, NX_U32 flags)
{
    if (!attr)
    {
        return NX_EINVAL;
    }
    attr->handler = handler;
    attr->flags = flags;
    return NX_EOK;
}

NX_Error NX_SignalGetAttr(NX_Signal signal, NX_SignalAttr * outAttr)
{
    NX_Thread * self;
    NX_SignalTable * signalTable;
    NX_SignalEntry * entry;
    NX_UArch level;

    if (signal <= NX_SIGNAL_INVALID || signal >= NX_SIGNAL_MAX || !outAttr)
    {
        return NX_EINVAL;
    }

    self = NX_ThreadSelf();

    signalTable = &self->resource.signals;
    entry = &signalTable->signalEntry[signal];

    NX_SpinLockIRQ(&entry->lock, &level);
    *outAttr = entry->attr;
    NX_SpinUnlockIRQ(&entry->lock, level);

    return NX_EOK;
}

NX_Error NX_SignalSetAttr(NX_Signal signal, NX_SignalAttr * attr)
{
    NX_Thread * self;
    NX_SignalTable * signalTable;
    NX_SignalEntry * entry;
    NX_UArch level;

    if (signal <= NX_SIGNAL_INVALID || signal >= NX_SIGNAL_MAX || !attr)
    {
        return NX_EINVAL;
    }

    /* kill and stop can not set attr */
    if (signal == NX_SIGNAL_KILL || signal == NX_SIGNAL_STOP)
    {
        return NX_EINVAL;
    }

    self = NX_ThreadSelf();

    signalTable = &self->resource.signals;
    entry = &signalTable->signalEntry[signal];

    NX_SpinLockIRQ(&entry->lock, &level);
    entry->attr = *attr;
    NX_SpinUnlockIRQ(&entry->lock, level);

    return NX_EOK;
}

NX_Error NX_SignalContorl(NX_Signal signalFirst, NX_Signal signalLast, NX_U32 cmd)
{
    NX_Thread * self;
    NX_SignalTable * signalTable;
    NX_SignalEntry * entry;
    NX_Signal signal;
    NX_UArch level;

    if (signalFirst <= NX_SIGNAL_INVALID || signalFirst >= NX_SIGNAL_MAX)
    {
        return NX_EINVAL;
    }

    if (signalLast <= NX_SIGNAL_INVALID || signalLast >= NX_SIGNAL_MAX)
    {
        return NX_EINVAL;
    }

    if (signalFirst > signalLast)
    {
        return NX_EINVAL;
    }

    if (cmd != NX_SIGNAL_CMD_BLOCK && cmd != NX_SIGNAL_CMD_UNBLOCK)
    {
        return NX_EINVAL;
    }

    for (signal = signalFirst; signal <= signalLast; signal++)
    {
        /* kill and stop can not control */
        if (signal == NX_SIGNAL_KILL || signal == NX_SIGNAL_STOP)
        {
            return NX_EINVAL;
        }
    }

    self = NX_ThreadSelf();
    signalTable = &self->resource.signals;

    for (signal = signalFirst; signal <= signalLast; signal++)
    {
        entry = &signalTable->signalEntry[signal];

        NX_SpinLockIRQ(&entry->lock, &level);
        if (cmd == NX_SIGNAL_CMD_BLOCK)
        {
            entry->blocked = NX_True;
        }
        else if (cmd == NX_SIGNAL_CMD_UNBLOCK)
        {
            entry->blocked = NX_False;
        }
        NX_SpinUnlockIRQ(&entry->lock, level);
    }
    
    return NX_EOK;
}

NX_PRIVATE NX_Error AddSignalEntry(NX_SignalTable * signalTable, NX_U32 tid, NX_Signal signal, void * signalValue)
{
    NX_SignalInfoEntry * infoEntry;
    NX_SignalEntry * entry;
    NX_UArch level;

    if ((infoEntry = NX_MemAllocEx(NX_SignalInfoEntry)) == NX_NULL)
    {
        return NX_ENOMEM;
    }

    infoEntry->info.signal = signal;
    infoEntry->info.tid = tid;
    infoEntry->info.signalValue = signalValue;

    entry = &signalTable->signalEntry[signal];

    NX_SpinLockIRQ(&entry->lock, &level);
    NX_ListAddTail(&infoEntry->list, &entry->signalInfoListHead);
    NX_AtomicInc(&entry->pendingSignals);
    NX_AtomicInc(&signalTable->globalPendingSignals);
    NX_SpinUnlockIRQ(&entry->lock, level);
    return NX_EOK;
}

NX_PRIVATE void RemoveSignalEntry(NX_SignalTable * signalTable, NX_SignalEntry * entry, NX_SignalInfo * outInfoEntry)
{
    NX_SignalInfoEntry * infoEntry;
    NX_UArch level;

    NX_SpinLockIRQ(&entry->lock, &level);
    infoEntry = NX_ListFirstEntryOrNULL(&entry->signalInfoListHead, NX_SignalInfoEntry, list);
    NX_ASSERT(infoEntry);
    NX_ListDel(&infoEntry->list);
    NX_AtomicDec(&entry->pendingSignals);
    NX_AtomicDec(&signalTable->globalPendingSignals);
    NX_SpinUnlockIRQ(&entry->lock, level);
    
    *outInfoEntry = infoEntry->info;

    NX_MemFree(infoEntry);
}

/**
 * @brief send signal to thead, the thread must in process!
 *  
 * @param tid thread id
 * @param signal signal
 * @param signalValue signal value
 * @return NX_Error 
 */
NX_Error NX_SignalSend(NX_U32 tid, NX_Signal signal, void * signalValue)
{
    NX_Thread * thread;
    NX_Process * process;
    NX_SignalTable * signalTable;
    NX_SignalEntry * entry;
    NX_Error err;

    NX_LOG_D("signal send, tid:%d, signal:%d, signal value:%p", tid, signal, signalValue);

    if (signal <= NX_SIGNAL_INVALID || signal >= NX_SIGNAL_MAX)
    {
        return NX_EINVAL;
    }

    if ((thread = NX_ThreadFindById(tid)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    if ((process = NX_ThreadGetProcess(thread)) == NX_NULL) /* no process */
    {
        return NX_ENORES;
    }

    signalTable = &thread->resource.signals;
    entry = &signalTable->signalEntry[signal];

    if (entry->blocked == NX_False)
    {
        /* send signal to signal info list */
        if ((err = AddSignalEntry(signalTable, tid, signal, signalValue)) != NX_EOK)
        {
            return err;
        }
        NX_ThreadUnblock(thread); /* wakeup thread if blocked */
    }

    return NX_EOK;
}

NX_PRIVATE void HandleSignal(NX_Thread * thread, NX_SignalTable * signalTable, NX_SignalEntry * entry, NX_SignalInfo * info)
{
    /* check ignore */
    if ((entry->attr.handler == NX_SIGNAL_HANDLER_DEFAULT && info->signal == NX_SIGNAL_CONTINUE) || 
        entry->attr.handler == NX_SIGNAL_HANDLER_IGNORE)
    {
        NX_LOG_D("IGNORE signal:%d", info->signal);
        return;
    }

    /* check default */
    if (entry->attr.handler == NX_SIGNAL_HANDLER_DEFAULT)
    {
        switch (info->signal)
        {
        case NX_SIGNAL_IRQ:
            /* TODO: handle IRQ */
            NX_LOG_W("handle IRQ signal.");
            break;
        case NX_SIGNAL_STOP:
            NX_LOG_D("STOP signal:%d", info->signal);
            NX_ThreadBlock(thread);
            break;
        default: /* kill thread */
            NX_LOG_D("KILL signal:%d", info->signal);
            NX_ThreadExit(0);
            break;
        }
    }
    else /* call user handler */
    {
        NX_LOG_W("handle user handler: %p", entry->attr.handler);
    }
}

void NX_SignalCheck(void)
{
    NX_Thread * self;
    NX_SignalTable * signalTable;
    NX_SignalEntry * entry;
    NX_Signal signal;
    NX_SignalInfo info;
    NX_UArch level;

    self = NX_ThreadSelf();

    signalTable = &self->resource.signals;

    if (!NX_AtomicGet(&signalTable->globalPendingSignals)) /* no signal */
    {
        return;
    }

    NX_LOG_D("thread:%d/%s has signal!", self->tid, self->name);

    level = NX_IRQ_SaveLevel();
    for (signal = (NX_SIGNAL_INVALID + 1); signal < NX_SIGNAL_MAX; signal++)
    {
        entry = &signalTable->signalEntry[signal];
        if (NX_AtomicGet(&entry->pendingSignals) > 0)
        {
            NX_LOG_D("handle signal:%d", signal);

            RemoveSignalEntry(signalTable, entry, &info);
            /* handle signal */
            HandleSignal(self, signalTable, entry, &info);
        }
    }
    NX_IRQ_RestoreLevel(level);
}
