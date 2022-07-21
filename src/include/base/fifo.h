/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: fifo buffer
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-06-05     JasonHu           Init
 */

#ifndef __UTILS_FIFO_BUF_H__
#define __UTILS_FIFO_BUF_H__

#include <nxos.h>
#include <base/spin.h>

typedef struct NX_Fifo
{
    NX_U8 * buffer;
    NX_Size size;
    NX_Size in;
    NX_Size out;
    NX_Spin lock;
} NX_Fifo;

NX_Error NX_FifoInit(NX_Fifo * fifo, NX_U8 * buffer, NX_Size size);
NX_Fifo * NX_FifoCreate(NX_Size size);
void NX_FifoDestroy(NX_Fifo * fifo);

NX_Size NX_FifoPut(NX_Fifo * fifo, const NX_U8 * buffer, NX_Size len);
NX_Size NX_FifoGet(NX_Fifo * fifo, const NX_U8 * buffer, NX_Size len);

void NX_FifoReset(NX_Fifo *fifo);
NX_Size NX_FifoLen(NX_Fifo *fifo);
NX_Size NX_FifoAvaliable(NX_Fifo *fifo);

#endif /* __UTILS_FIFO_BUF_H__ */
