/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Clock manage
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-31     JasonHu           Init
 */

#ifndef __MODS_TIME_CLOCK__
#define __MODS_TIME_CLOCK__

#include <nxos.h>

#ifdef CONFIG_NX_TICKS_PER_SECOND
#define NX_TICKS_PER_SECOND CONFIG_NX_TICKS_PER_SECOND
#else
#define NX_TICKS_PER_SECOND 100
#endif

#if (NX_TICKS_PER_SECOND > 1000) || (NX_TICKS_PER_SECOND == 0)
#error "config ticks range in [1~1000]"
#endif

#define NX_TICKS_PER_MILLISECOND (1000 / NX_TICKS_PER_SECOND)
#define NX_MILLISECOND_TO_TICKS(msec) ((msec) / NX_TICKS_PER_MILLISECOND)
#define NX_TICKS_TO_MILLISECOND(ticks) ((ticks) * NX_TICKS_PER_MILLISECOND)

typedef NX_UArch NX_TimeVal;
typedef NX_UArch NX_ClockTick;

NX_ClockTick NX_ClockTickGet(void);
void NX_ClockTickSet(NX_ClockTick tick);
void NX_ClockTickGo(void);

NX_Error NX_ClockTickDelay(NX_ClockTick ticks);

NX_INLINE NX_TimeVal NX_ClockTickToMillisecond(NX_ClockTick tick)
{
    return NX_TICKS_TO_MILLISECOND(tick);
}

NX_INLINE NX_ClockTick NX_MillisecondToClockTick(NX_TimeVal milliseconds)
{
    return NX_MILLISECOND_TO_TICKS(milliseconds);
}

NX_INLINE NX_TimeVal NX_ClockTickGetMillisecond(void)
{
    return NX_ClockTickToMillisecond(NX_ClockTickGet());
}

NX_INLINE NX_Error NX_ClockTickDelayMillisecond(NX_TimeVal milliseconds)
{
    return NX_ClockTickDelay(NX_MillisecondToClockTick(milliseconds));
}

NX_Error NX_ClockInit(void);

#endif  /* __MODS_TIME_CLOCK__ */
