/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: auto init
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-2     JasonHu           Init
 */

#ifndef __XBOOK_INIT_CALL__
#define __XBOOK_INIT_CALL__

#include <nxos.h>

typedef void (*NX_InitCallHandler)(void);
typedef void (*NX_ExitCallHandler)(void);

#define __DEFINE_INITCALL(level, fn, id) \
	NX_PRIVATE const NX_InitCallHandler __NX_initCall##fn##Id \
	__attribute__((__used__, __section__(".initCall" level ".text"))) = fn

#define __DEFINE_EXITCALL(level, fn, id) \
	NX_PRIVATE const NX_ExitCallHandler __NX_ExitCall##fn##Id \
	__attribute__((__used__, __section__(".exitCall" level ".text"))) = fn

#define NX_INITCALL(fn)		        __DEFINE_INITCALL("0", fn, 0)
#define NX_EXITCALL(fn)		        __DEFINE_EXITCALL("0", fn, 0)

#define NX_MODS_INIT(fn)            __DEFINE_INITCALL("1", fn, 1)
#define NX_MODS_EXIT(fn)            __DEFINE_EXITCALL("1", fn, 1)

#define NX_DRV_INIT(fn)            	__DEFINE_INITCALL("2", fn, 2)
#define NX_DRV_EXIT(fn)            	__DEFINE_EXITCALL("2", fn, 2)

#define NX_FS_INIT(fn)            	__DEFINE_INITCALL("3", fn, 3)
#define NX_FS_EXIT(fn)            	__DEFINE_EXITCALL("3", fn, 3)

#define NX_FINAL_INIT(fn)     		__DEFINE_INITCALL("8", fn, 8)
#define NX_FINAL_EXIT(fn)           __DEFINE_EXITCALL("8", fn, 8)

#define NX_INIT_TEST(fn)            __DEFINE_INITCALL("9", fn, 9)

void NX_CallInvoke(NX_InitCallHandler start[], NX_InitCallHandler end[]);

void NX_InitCallInvoke(void);
void NX_ExitCallInvoke(void);

void NX_CallsInit(void);

#endif  /* __XBOOK_INIT_CALL__ */
