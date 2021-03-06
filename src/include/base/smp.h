/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Symmetrical Multi-Processing
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-12-10     JasonHu           Init
 */

#ifndef __SCHED_SMP__
#define __SCHED_SMP__

#include <nxos.h>
#include <base/list.h>
#include <base/thread.h>
#include <base/spin.h>
#include <base/atomic.h>

struct NX_Cpu
{
    NX_List threadReadyList[NX_THREAD_MAX_PRIORITY_NR];   /* list for thread ready to run */
    NX_Thread *threadRunning;  /* the thread running on core */
    NX_Thread *idleThread;  /* the idle thread on core */
    NX_ClockTick idleElapsedTicks;
    NX_U32 idleTime;

    NX_Spin lock;     /* lock for CPU */
    NX_Atomic threadCount;    /* ready thread count on this core */
};
typedef struct NX_Cpu NX_Cpu;

struct NX_SMP_Ops
{
    NX_UArch (*getIdx)(void);
    NX_Error (*bootApp)(NX_UArch bootCoreId);
    NX_Error (*enterApp)(NX_UArch appCoreId);
};

NX_INTERFACE NX_IMPORT struct NX_SMP_Ops NX_SMP_OpsInterface; 

#define NX_SMP_BootApp(bootCoreId)  NX_SMP_OpsInterface.bootApp(bootCoreId)
#define NX_SMP_EnterApp(appCoreId)  NX_SMP_OpsInterface.enterApp(appCoreId)
#define NX_SMP_GetIdx()             NX_SMP_OpsInterface.getIdx()

void NX_SMP_Preload(NX_UArch coreId);
void NX_SMP_Init(NX_UArch coreId);
void NX_SMP_Main(NX_UArch coreId);
void NX_SMP_Stage2(NX_UArch appCoreId);

NX_UArch NX_SMP_GetBootCore(void);

void NX_SMP_EnqueueThreadIrqDisabled(NX_UArch coreId, NX_Thread *thread, int flags);
void NX_SMP_DequeueThreadIrqDisabled(NX_UArch coreId, NX_Thread *thread);

void NX_SMP_DequeueThread(NX_UArch coreId, NX_Thread *thread);

NX_Thread *NX_SMP_PickThreadIrqDisabled(NX_UArch coreId);
NX_Error NX_SMP_SetRunning(NX_UArch coreId, NX_Thread *thread);

NX_Cpu *NX_CpuGetIndex(NX_UArch coreId);

NX_Thread *NX_SMP_DeququeNoAffinityThread(NX_UArch coreId);

/**
 * get CPU by core id
 */
NX_INLINE NX_Cpu *NX_CpuGetPtr(void)
{
    return NX_CpuGetIndex(NX_SMP_GetIdx());
}

NX_Thread *NX_SMP_GetRunning(void);
NX_Error NX_SMP_SetIdle(NX_UArch coreId, NX_Thread *thread);
NX_Thread * NX_SMP_GetIdle(NX_UArch coreId);
NX_U32 NX_SMP_GetUsage(NX_UArch coreId);

#endif /* __SCHED_SMP__ */
