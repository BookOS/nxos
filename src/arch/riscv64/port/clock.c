/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Clock for system 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-16     JasonHu           Init
 */

#include <time/clock.h>
#include <io/irq.h>
#include <io/delay_irq.h>

#include <clock.h>
#include <regs.h>
#include <sbi.h>

#define NX_LOG_NAME "Clock"
#include <utils/log.h>

#define NX_TIMER_CLK_FREQ 10000000  /* qemu has 10MHZ clock frequency */

NX_PRIVATE NX_U64 TickDelta = NX_TIMER_CLK_FREQ / NX_TICKS_PER_SECOND;

NX_PRIVATE NX_U64 GetTimerCounter()
{
    NX_U64 ret;
    NX_CASM ("rdtime %0" : "=r"(ret));
    return ret;
}

void NX_HalClockHandler(void)
{
    NX_ClockTickGo();
    /* update timer */
    sbi_set_timer(GetTimerCounter() + TickDelta);
}

NX_INTERFACE NX_Error NX_HalInitClock(void)
{
    /* Clear the Supervisor-Timer bit in SIE */
    ClearCSR(sie, SIE_STIE);

    /* Set timer */
    sbi_set_timer(GetTimerCounter() + TickDelta);

    /* Enable the Supervisor-Timer bit in SIE */
    SetCSR(sie, SIE_STIE);
    return NX_EOK;
}
