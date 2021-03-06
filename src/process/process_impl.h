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

#include <base/process.h>
#include <base/thread.h>

NX_Error NX_ProcessUnmapTls(NX_Process * process, NX_Thread * thread);
NX_Error NX_ProcessMapTls(NX_Process * process, NX_Thread * thread);

void NX_ProcessAppendThread(NX_Process *process, void *thread);
void NX_ProcessDeleteThread(NX_Process *process, NX_Thread *thread);

#endif /* __PROCESS_IMPL_H__ */
