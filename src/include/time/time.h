/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Timer for system
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-05-14     JasonHu           Init
 */

#ifndef __TIME_TIME_H__
#define __TIME_TIME_H__

#include <xbook.h>

typedef struct {
    NX_U8 second;       /* [0-59] */
    NX_U8 minute;       /* [0-59] */
    NX_U8 hour;         /* [0-23] */
    NX_U8 weekDay;      /* [0-6] */
    NX_U32 day;         /* [1-31] */
    NX_U32 month;       /* [1-12] */
    NX_U32 year;        /* year */
    NX_U32 yearDay;     /* [0-366] */
} NX_Time;

#define NX_TIME_INIT_VAL {0, 0, 0, 0, 0, 1, 1980, 0}

#define NX_TIME_DEFINE(time) NX_Time time = NX_TIME_INIT_VAL;

void NX_TimeGo(void);
void NX_TimePrint(void);
NX_Error NX_TimeSet(NX_Time * time);
NX_Error NX_TimeGet(NX_Time * time);

#endif  /* __TIME_TIME_H__ */
