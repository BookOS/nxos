/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Block device
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-3-20      JasonHu           Init
 */

#ifndef __IO_BLOCK__
#define __IO_BLOCK__

#include <nxos.h>
#include <base/driver.h>

#define NX_IO_BLOCK_INFO 1
#define NX_IO_BLOCK_SYNC 2

typedef struct NX_IoBlockInfo
{
    NX_Size blockSize;
    NX_Size blockCount;
    NX_Size capacity;
} NX_IoBlockInfo;

NX_Error NX_IoBlockSearch(const char *name);

NX_Device *NX_IoBlockOpen(const char *name);
NX_Error NX_IoBlockClose(NX_Device *blk);
NX_Size NX_IoBlockCapacity(NX_Device *blk);
NX_Size NX_IoBlockRead(NX_Device *blk, NX_U8 * buf, NX_Offset offset, NX_Size count);
NX_Size NX_IoBlockWrite(NX_Device *blk, NX_U8 * buf, NX_Offset offset, NX_Size count);
void NX_IoBlockSync(NX_Device *blk);

NX_INLINE NX_Size NX_IoBlockAvailable(NX_Device *blk, NX_Offset offset, NX_Size length)
{
	NX_Size cap;

	if(blk)
	{
		cap = NX_IoBlockCapacity(blk);
		if(offset + length > cap)
			return cap - offset;
		return length;
	}
	return 0;
}

#endif  /* __IO_BLOCK__ */
