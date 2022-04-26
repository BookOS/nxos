/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: process env
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-26      JasonHu           Init
 */

#ifndef __PROCESS_ENV_H__
#define __PROCESS_ENV_H__

#include <xbook.h>

#ifdef CONFIG_NX_PORCESS_ENV_ARGS
#define NX_PORCESS_ENV_ARGS CONFIG_NX_PORCESS_ENV_ARGS
#else
#define NX_PORCESS_ENV_ARGS 1024
#endif

int NX_EnvToArray(char * buf, char * argArray[], int maxArgs);
int NX_CmdToArray(char * buf, char * argArray[], int maxArgs);
int NX_EnvToBuf(char * buf, int bufLen, char * argArray[]);
int NX_CmdToBuf(char * buf, int bufLen, char * argArray[]);

#endif  /* __PROCESS_ENV_H__ */
