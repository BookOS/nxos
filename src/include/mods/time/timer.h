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

#ifndef __MODS_TIME_TIMER__
#define __MODS_TIME_TIMER__

#include <xbook.h>
#include <utils/list.h>
#include <mods/time/clock.h>

/* 24 days */
#define NX_MAX_TIMER_TIMEOUT (24 * 24 * 60 * 60 * 1000)

#define NX_TIMER_ONESHOT   0x01    /* timer type is one shot */
#define NX_TIMER_PERIOD    0x02    /* timer type is period */
#define NX_TIMER_DYNAMIC   0x08    /* timer create from memory heap */

enum NX_TimerState
{
    NX_TIMER_INITED = 1,   /* init, not run */
    NX_TIMER_WAITING,      /* waiting for timeout */
    NX_TIMER_PROCESSING,   /* timer processing */
    NX_TIMER_STOPPED,      /* timer stopped */
};
typedef enum NX_TimerState NX_TimerState;

struct NX_Timer
{
    NX_List list;
    NX_TimerState state;   /* timer state */
    NX_ClockTick timeout;  /* timeout ticks */ 
    NX_ClockTick timeTicks;
    int flags;
    NX_Bool (*handler)(struct NX_Timer *, void *arg);
    void *arg;
};
typedef struct NX_Timer NX_Timer;

NX_Error NX_TimerInit(NX_Timer *timer, NX_UArch milliseconds, 
                          NX_Bool (*handler)(struct NX_Timer *, void *arg), void *arg, 
                          int flags);
                         
NX_Timer *NX_TimerCreate(NX_UArch milliseconds, 
                          NX_Bool (*handler)(struct NX_Timer *, void *arg), void *arg, 
                          int flags);
                           
NX_Error NX_TimerStart(NX_Timer *timer);
NX_Error NX_TimerStop(NX_Timer *timer);
NX_Error NX_TimerDestroy(NX_Timer *timer);

void NX_TimerDump(NX_Timer *timer);

void NX_TimersInit(void);
void NX_TimerGo(void);

#endif  /* __MODS_TIME_TIMER__ */
