/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: io queue
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-21      JasonHu           Init
 */

#ifndef __IO_QUEUE_H__
#define __IO_QUEUE_H__

#include <sched/semaphore.h>

typedef struct NX_IoQueue
{
    NX_Semaphore full; /* full solt sem */
    NX_Semaphore empty; /* empty solt sem */
    NX_Semaphore mutex;
    NX_Size length;
    NX_Offset getIndex;
    NX_Offset putIndex;
    char * buf;
} NX_IoQueue;

NX_IoQueue *NX_IoQueueCreate(NX_Size length, NX_Error * outErr);
NX_Error NX_IoQueueDestroy(NX_IoQueue * queue);

NX_Error NX_IoQueueInit(NX_IoQueue * queue, char * buf, NX_Size length);

char NX_IoQueueGet(NX_IoQueue * queue, NX_Error * outErr);
NX_Error NX_IoQueuePut(NX_IoQueue * queue, char data);

char NX_IoQueueTryGet(NX_IoQueue * queue, NX_Error *outErr);
NX_Error NX_IoQueueTryPut(NX_IoQueue * queue, char data);

#endif  /* __IO_QUEUE_H__ */
