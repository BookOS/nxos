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

#if defined(CONFIG_NX_PLATFORM_D1)

#define NX_TIMER_CLK_FREQ (24000000) /* 24MHZ */

#elif defined(CONFIG_NX_PLATFORM_RISCV64_QEMU) || \
      defined(CONFIG_NX_PLATFORM_K210)
#define NX_TIMER_CLK_FREQ (10000000) /* 10MHZ*/

#elif defined(CONFIG_NX_PLATFORM_HIFIVE_UNMACHED)

#define NX_TIMER_CLK_FREQ (1000000) /* 1MHZ*/

#else
#error "no clock frequency"
#endif

NX_PRIVATE NX_U64 tickDelta = NX_TIMER_CLK_FREQ / NX_TICKS_PER_SECOND;

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
    sbi_set_timer(GetTimerCounter() + tickDelta);
}

NX_INTERFACE NX_Error NX_HalInitClock(void)
{
    /* Clear the Supervisor-Timer bit in SIE */
    ClearCSR(sie, SIE_STIE);

    /* Set timer */
    sbi_set_timer(GetTimerCounter() + tickDelta);

    /* Enable the Supervisor-Timer bit in SIE */
    SetCSR(sie, SIE_STIE);
    return NX_EOK;
}
