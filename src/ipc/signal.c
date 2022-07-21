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
#define NX_LOG_NAME "signal"
#define NX_LOG_LEVEL NX_LOG_INFO
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
    NX_AtomicSet(&table->userHandled, 0);
    return NX_EOK;
}

NX_Error NX_SignalTableExit(NX_SignalTable * table)
{
    int i;
    NX_SignalEntry * entry;
    NX_SignalInfoEntry * infoEntry, * infoEntryNext;
    NX_UArch level;

    if (!table)
    {
        return NX_EINVAL;
    }
    
    NX_Thread * self = NX_ThreadSelf();
    
    for (i = 0; i < NX_SIGNAL_MAX; i++)
    {
        entry = &table->signalEntry[i];
        if (NX_AtomicGet(&entry->pendingSignals) > 0)
        {
            self->flags |= NX_THREAD_FLAG_UNINTERRUPTABLE;
            NX_SpinLockIRQ(&entry->lock, &level);
            self->flags &= ~NX_THREAD_FLAG_UNINTERRUPTABLE;

            NX_ListForEachEntrySafe (infoEntry, infoEntryNext, &entry->signalInfoListHead, list)
            {
                NX_ListDel(&infoEntry->list);
                NX_MemFree(infoEntry);
            }
            NX_SpinUnlockIRQ(&entry->lock, level);
            
            NX_AtomicSet(&entry->pendingSignals, 0);
            entry->blocked = NX_False;
        }
    }
    NX_AtomicSet(&table->globalPendingSignals, 0);
    NX_AtomicSet(&table->userHandled, 0);
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

    NX_Error err;
    if ((err = NX_SpinLockIRQ(&entry->lock, &level)) != NX_EOK)
    {
        return err;
    }
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

    NX_Error err;
    if ((err = NX_SpinLockIRQ(&entry->lock, &level)) != NX_EOK)
    {
        return err;
    }
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
        self->flags |= NX_THREAD_FLAG_UNINTERRUPTABLE;
        NX_SpinLockIRQ(&entry->lock, &level);
        self->flags &= ~NX_THREAD_FLAG_UNINTERRUPTABLE;
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

NX_PRIVATE NX_Error AddSignalEntry(NX_SignalTable * signalTable, NX_U32 tid, NX_Signal signal, void * signalValue, NX_SignalExtralInfo * extralInfo)
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
    if (extralInfo != NX_NULL)
    {
        infoEntry->info.extralInfo = *extralInfo;
    }

    entry = &signalTable->signalEntry[signal];

    NX_Error err;
    if ((err = NX_SpinLockIRQ(&entry->lock, &level)) != NX_EOK)
    {
        return err;
    }
    NX_ListAddTail(&infoEntry->list, &entry->signalInfoListHead);
    NX_AtomicInc(&entry->pendingSignals);
    NX_AtomicInc(&signalTable->globalPendingSignals);
    NX_SpinUnlockIRQ(&entry->lock, level);
    return NX_EOK;
}

/* return 0 ignore signal, return 1 handle signal, return -1 skip signal */
NX_PRIVATE int CheckSignalHandlable(NX_SignalTable * signalTable, NX_SignalEntry * entry, NX_SignalInfoEntry * infoEntry, void * trapframe)
{
    /* check ignore */
    if ((entry->attr.handler == NX_SIGNAL_HANDLER_DEFAULT && infoEntry->info.signal == NX_SIGNAL_CONTINUE) || 
        entry->attr.handler == NX_SIGNAL_HANDLER_IGNORE)
    {
        return 0;
    }

    /* check default */
    if (entry->attr.handler == NX_SIGNAL_HANDLER_DEFAULT)
    {
        return 1;
    }

    /* check handle user */
    if (NX_AtomicGet(&signalTable->userHandled) > 0 || trapframe == NX_NULL) /* user handled, can not handle again! */
    {
        return -1;
    }
    /* handle user signal */
    return 1;
}

NX_PRIVATE NX_Bool RemoveSignalEntry(NX_SignalTable * signalTable, NX_SignalEntry * entry, NX_SignalInfo * outInfoEntry, void * trapframe)
{
    NX_SignalInfoEntry * infoEntry;
    NX_UArch level;
    int checkSignal;

    NX_Error err;
    if ((err = NX_SpinLockIRQ(&entry->lock, &level)) != NX_EOK)
    {
        return err;
    }
    infoEntry = NX_ListFirstEntryOrNULL(&entry->signalInfoListHead, NX_SignalInfoEntry, list);
    NX_ASSERT(infoEntry);
    
    /* check info */
    if ((checkSignal = CheckSignalHandlable(signalTable, entry, infoEntry, trapframe)) < 0)
    {
        NX_SpinUnlockIRQ(&entry->lock, level);
        return NX_False;
    }
    
    /* remove info entry */
    NX_ListDel(&infoEntry->list);
    NX_AtomicDec(&entry->pendingSignals);
    NX_AtomicDec(&signalTable->globalPendingSignals);
    NX_SpinUnlockIRQ(&entry->lock, level);
    
    *outInfoEntry = infoEntry->info;

    NX_MemFree(infoEntry);

    if (checkSignal == 0) /* ignore signal do not handle */
    {
        return NX_False;
    }

    return NX_True;
}

NX_PRIVATE NX_Error SendSignalToThread(NX_Thread * thread, NX_U32 senderTid, NX_Signal signal, void * signalValue, NX_SignalExtralInfo * extralInfo)
{
    NX_SignalTable * signalTable;
    NX_SignalEntry * entry;
    NX_Error err;

    signalTable = &thread->resource.signals;
    entry = &signalTable->signalEntry[signal];

    if (entry->blocked == NX_False)
    {
        /* send signal to signal info list */
        if ((err = AddSignalEntry(signalTable, senderTid, signal, signalValue, extralInfo)) != NX_EOK)
        {
            return err;
        }
        thread->flags |= NX_THREAD_FLAG_SIGNAL_INTR;
        NX_ThreadUnblock(thread); /* wakeup thread if blocked */
    }
    return NX_EOK;
}

NX_PRIVATE NX_Bool HandleUserSignal(NX_Thread * thread, NX_SignalTable * signalTable, NX_SignalEntry * entry, NX_SignalInfo * info, void * trapframe)
{
    /* check default */
    if (entry->attr.handler == NX_SIGNAL_HANDLER_DEFAULT)
    {
        switch (info->signal)
        {
        case NX_SIGNAL_IRQ:
            /* TODO: handle IRQ */
            NX_LOG_D("handle IRQ signal in thread %d", thread->tid);
            break;
        case NX_SIGNAL_STOP:
            NX_LOG_D("STOP thread %d with signal %d", thread->tid, info->signal);
            NX_ThreadBlock(thread);
            break;
        default: /* kill thread */
            NX_LOG_D("KILL thread %d with signal %d", thread->tid, info->signal);
            NX_ThreadExit(0);
            NX_PANIC("kill thread with signal failed!");
            break;
        }
        return NX_False;
    }
    else /* call user handler */
    {
        NX_AtomicInc(&signalTable->userHandled);
        NX_ProcessHandleSignal(trapframe, info, entry);
        return NX_True;
    }
}

/**
 * @brief send signal to thead, the thread must in process!
 *  
 * @param tid thread id
 * @param signal signal
 * @param signalValue signal value
 * @return NX_Error 
 */
NX_Error NX_SignalSend(NX_U32 tid, NX_Signal signal, void * signalValue, NX_SignalExtralInfo * extralInfo)
{
    NX_Thread * thread;

    NX_LOG_D("signal send, tid:%d, signal:%d, signal value:%p", tid, signal, signalValue);

    if (signal <= NX_SIGNAL_INVALID || signal >= NX_SIGNAL_MAX)
    {
        return NX_EINVAL;
    }

    if ((thread = NX_ThreadFindById(tid)) == NX_NULL)
    {
        return NX_ENOSRCH;
    }

    return SendSignalToThread(thread, tid, signal, signalValue, extralInfo);
}

void NX_SignalExitHandleUser(void)
{
    NX_Thread * self;
    NX_SignalTable * signalTable;

    self = NX_ThreadSelf();
    signalTable = &self->resource.signals;
    NX_AtomicDec(&signalTable->userHandled);
}

void NX_SignalCheck(void * trapframe)
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

    level = NX_IRQ_SaveLevel();
    
    self->flags &= ~NX_THREAD_FLAG_SIGNAL_INTR; /* clear intr flag */

    for (signal = (NX_SIGNAL_INVALID + 1); signal < NX_SIGNAL_MAX; signal++)
    {
        entry = &signalTable->signalEntry[signal];
        if (NX_AtomicGet(&entry->pendingSignals) > 0)
        {
            if (RemoveSignalEntry(signalTable, entry, &info, trapframe) == NX_False)
            {
                continue; /* fetch signal ignore or can not handle user */
            }
            /* handle signal */
            if (HandleUserSignal(self, signalTable, entry, &info, trapframe) == NX_True)
            {
                break; /* handle user need exit signal check */
            }
        }
    }
    NX_IRQ_RestoreLevel(level);
}

NX_Size NX_SignalCurrentPending(void)
{
    NX_Thread * self;

    self = NX_ThreadSelf();
    return (NX_Size)NX_AtomicGet(&self->resource.signals.globalPendingSignals);
}
