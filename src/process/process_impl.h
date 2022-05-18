/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Process imple
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-05-19     JasonHu           Init
 */

#ifndef __PROCESS_IMPL_H__
#define __PROCESS_IMPL_H__

#include <process/process.h>
#include <sched/thread.h>

NX_Error NX_ProcessUnmapTls(NX_Process * process, NX_Thread * thread);
NX_Error NX_ProcessMapTls(NX_Process * process, NX_Thread * thread);

#endif /* __PROCESS_IMPL_H__ */
