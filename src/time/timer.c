/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Timer for system
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-20     JasonHu           Init
 */

#include <time/timer.h>
#include <mm/alloc.h>

#include <utils/log.h>
#include <xbook/debug.h>
#include <sched/spin.h>

#define NX_IDLE_TIMER_TIMEOUT  NX_MAX_TIMER_TIMEOUT
#define NX_IDLE_TIMER_TIMEOUT_TICKS  (NX_IDLE_TIMER_TIMEOUT / (1000 / NX_TICKS_PER_SECOND))

NX_PRIVATE NX_LIST_HEAD(timerListHead);

/* timer tick is different with clock tick */
NX_PRIVATE NX_VOLATILE NX_ClockTick timerTicks = 0;

/* next timeout tick */
NX_PRIVATE NX_VOLATILE NX_ClockTick nextTimeoutTicks = 0;

NX_PRIVATE NX_Timer idleTimer;

NX_PRIVATE NX_Spin timersLock;

NX_Error NX_TimerInit(NX_Timer *timer, NX_UArch milliseconds, 
                          NX_Bool (*handler)(struct NX_Timer *, void *arg), void *arg, 
                          int flags)
{
    if (timer == NX_NULL || !milliseconds || handler == NX_NULL || flags == 0)
    {
        return NX_EINVAL;
    }

    if (!(flags & (NX_TIMER_ONESHOT | NX_TIMER_PERIOD)) || (flags & NX_TIMER_DYNAMIC))
    {
        return NX_EINVAL;
    }

    timer->flags = flags;
    timer->state = NX_TIMER_INITED;

    timer->timeTicks = NX_MILLISECOND_TO_TICKS(milliseconds);
    
    timer->timeout = 0;
    
    timer->handler = handler;
    timer->arg = arg;
    NX_ListInit(&timer->list);
    return NX_EOK;
}

NX_Timer *NX_TimerCreate(NX_UArch milliseconds, 
                          NX_Bool (*handler)(struct NX_Timer *, void *arg), void *arg, 
                          int flags)
{
    NX_Timer *timer = NX_MemAlloc(sizeof(NX_Timer));
    if (timer == NX_NULL)
    {
        return NX_NULL;
    }
    if (NX_TimerInit(timer, milliseconds, handler, arg, flags) != NX_EOK)
    {
        NX_MemFree(timer);
        return NX_NULL;
    }
    timer->flags |= NX_TIMER_DYNAMIC;
    return timer;
}

NX_PRIVATE void NX_TimerRemove(NX_Timer *timer, NX_Bool onTimerList, NX_Bool destroy)
{
    if (onTimerList == NX_True)
    {
        /* del from list */
        NX_ListDel(&timer->list);

        /* update next time */
        NX_Timer *next;
        NX_ListForEachEntry(next, &timerListHead, list)
        {
            if (next->timeout > timerTicks)
            {
                break;
            }
        }
        nextTimeoutTicks = next->timeout;
    }
    if (destroy == NX_True)
    {
        /* free timer */
        if (timer->flags & NX_TIMER_DYNAMIC)
        {
            NX_MemFree(timer);
        }
    }
}

/**
 * destroy a timer, timer must stopped or inited, not waiting and processing.
 */
NX_Error NX_TimerDestroy(NX_Timer *timer)
{
    if (timer == NX_NULL)
    {
        return NX_EINVAL;
    }
    switch (timer->state)
    {
    case NX_TIMER_WAITING:
    case NX_TIMER_PROCESSING:
        return NX_EAGAIN;
    case NX_TIMER_STOPPED:
    case NX_TIMER_INITED:
        {
            NX_UArch level;
            NX_SpinLockIRQ(&timersLock, &level);
            NX_TimerRemove(timer, NX_False, NX_True);
            NX_SpinUnlockIRQ(&timersLock, level);
        }
        break;
    default:
        return NX_EINVAL;
    }
    return NX_EOK;
}

NX_Error NX_TimerStart(NX_Timer *timer)
{
    if (timer == NX_NULL)
    {
        return NX_EINVAL;
    }

    NX_UArch level;

    NX_SpinLockIRQ(&timersLock, &level);

    /* calc timeout here */
    timer->timeout = timer->timeTicks + timerTicks;

    /* timeout is invalid */
    if (NX_IDLE_TIMER_TIMEOUT_TICKS - timer->timeTicks < timerTicks)
    {
        NX_SpinUnlockIRQ(&timersLock, level);
        return NX_EINVAL;
    }

    /* make sure not on the list */
    if (NX_ListFind(&timer->list, &timerListHead))
    {
        NX_SpinUnlockIRQ(&timersLock, level);
        return NX_EAGAIN;
    }
    
    /* waiting timeout state */
    timer->state = NX_TIMER_WAITING;
    if (NX_ListEmpty(&timerListHead))
    {
        /* inseart at head */
        NX_ListAdd(&timer->list, &timerListHead);
        nextTimeoutTicks = timer->timeout;
    }
    else
    {
        NX_Timer *first = NX_ListFirstEntry(&timerListHead, NX_Timer, list);
        if (timer->timeout < first->timeout)
        {
            /* insert at head */
            NX_ListAdd(&timer->list, &timerListHead);
            nextTimeoutTicks = timer->timeout;
        }
        else
        {
            /* insert after nearly timer */
            NX_Timer *prev = NX_NULL;
            NX_ListForEachEntryReverse(prev, &timerListHead, list)
            {
                if (timer->timeout >= prev->timeout)
                {
                    NX_ListAdd(&timer->list, &prev->list);
                    break;
                }
            }
        }
    }

    NX_SpinUnlockIRQ(&timersLock, level);
    return NX_EOK;
}

/**
 * only stop a timer, not destroy
 */
NX_PRIVATE NX_Error NX_TimerStopUnlocked(NX_Timer *timer)
{
    if (timer == NX_NULL)
    {
        return NX_EINVAL;
    }

    NX_TimerState state = timer->state;

    /* stop must when state is waiting or processing */
    if (state != NX_TIMER_PROCESSING && state != NX_TIMER_WAITING)
    {
        return NX_EAGAIN;
    }

    timer->state = NX_TIMER_STOPPED;

    /* direct del timer when waiting timer */
    if (state == NX_TIMER_WAITING)
    {
        NX_TimerRemove(timer, NX_True, NX_False);
    }

    return NX_EOK;
}

/**
 * only stop a timer, not destroy
 */
NX_Error NX_TimerStop(NX_Timer *timer)
{
    if (timer == NX_NULL)
    {
        return NX_EINVAL;
    }

    NX_Error err;
    NX_UArch level;
    NX_SpinLockIRQ(&timersLock, &level);

    err = NX_TimerStopUnlocked(timer);
    
    NX_SpinUnlockIRQ(&timersLock, level);
    return err;
}

NX_PRIVATE void NX_TimerInvoke(NX_Timer *timer)
{
    timer->state = NX_TIMER_PROCESSING;
    
    /* stop timer here */
    if (timer->handler(timer, timer->arg) == NX_False)
    {
        /* stop period timer if return false */
        if (timer->flags & NX_TIMER_PERIOD)
        {
            timer->state = NX_TIMER_STOPPED;
        }
    }

    /* when calling the handler, called stop timer, need stop here */
    if (timer->state == NX_TIMER_STOPPED)
    {
        NX_TimerRemove(timer, NX_True, NX_True);
    }
    else    /* always processing */
    {
        if (timer->flags & NX_TIMER_PERIOD)
        {
            /* update timer timeout */
            timer->timeout = timerTicks + timer->timeTicks;
            timer->state = NX_TIMER_WAITING;
        }
        else
        {
            timer->state = NX_TIMER_STOPPED;
            NX_TimerRemove(timer, NX_True, NX_True);
        }        
    }
    
}

/**
 * only master cpu will call this
 */
void NX_TimerGo(void)
{
    NX_Timer *timer = NX_NULL;
    NX_Timer *next = NX_NULL;
    
    timerTicks++;

    if (timerTicks < nextTimeoutTicks)
    {
        return;
    }

    NX_UArch level;
    
    NX_SpinLockIRQ(&timersLock, &level);

    NX_ListForEachEntrySafe(timer, next, &timerListHead, list)
    {
        if (timer->timeout > timerTicks) /* not timeout */
        {
            break;
        }
        /* timeout == timerTicks -> timeout! */
        NX_TimerInvoke(timer);
    }

    /* find next timer */
    NX_ListForEachEntry(timer, &timerListHead, list)
    {
        if (timer->timeout > timerTicks)
        {
            break;
        }
    }
    nextTimeoutTicks = timer->timeout;
    NX_SpinUnlockIRQ(&timersLock, level);
}

void NX_TimerDump(NX_Timer *timer)
{
    NX_LOG_I("==== NX_Timer ====");
    NX_LOG_I("addr:%p", timer);
    NX_LOG_I("state:%d", timer->state);
    NX_LOG_I("timeout:%p", timer->timeout);
    NX_LOG_I("timeTicks:%p", timer->timeTicks);
    NX_LOG_I("flags:%x", timer->flags);
    NX_LOG_I("handler:%p", timer->handler);
    NX_LOG_I("arg:%p", timer->arg);
}

/**
 * recalc all timers timeout
 * this will called with interrupt disabled
 */
NX_PRIVATE NX_Bool idleTimerHandler(NX_Timer *timer, void *arg)
{
    NX_ClockTick delta = idleTimer.timeout;
    timerTicks -= delta;
    NX_Timer *tmp;
    NX_ListForEachEntry (tmp, &timerListHead, list)
    {
        tmp->timeout -= delta;
    }
    return NX_True;
}

void NX_TimersInit(void)
{
    NX_SpinInit(&timersLock);
    NX_ASSERT(NX_TimerInit(&idleTimer, NX_IDLE_TIMER_TIMEOUT, idleTimerHandler, NX_NULL, NX_True) == NX_EOK);
    NX_ASSERT(NX_TimerStart(&idleTimer) == NX_EOK);
}
