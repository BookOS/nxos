/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Sched for NXOS
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-8      JasonHu           Init
 */

#ifndef __XBOOK_SCHED___
#define __XBOOK_SCHED___

#include <xbook.h>

#include <sched/spin.h>

#define NX_SCHED_HEAD          0x01
#define NX_SCHED_TAIL          0x02

void NX_SchedToFirstThread(void);
void NX_SchedInterruptDisabled(NX_UArch irqLevel);
void NX_SchedLockedIRQ(NX_UArch irqLevel, NX_Spin *lock);
void NX_SchedYield(void);
void NX_ReSchedCheck(void);
void NX_SchedExit(void);

#endif /* __XBOOK_SCHED___ */
