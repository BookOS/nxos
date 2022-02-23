/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Console output
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-1      JasonHu           Init
 */

#include <utils/log.h>
#include <xbook/debug.h>
#include <io/irq.h>

#ifdef CONFIG_NX_DEBUG
void NX_DebugAssertionFailure(char *exp, char *file, char *baseFile, int line)
{
    NX_LOG_E("\nAssert(%s) failed:\nfile: %s\nbase_file: %s\nln: %d", 
        exp, file, baseFile, line);
    NX_IRQ_Disable();
    while (1);
}
#endif

void NX_DebugSpin(const char *str)
{
    NX_LOG_I(str, NX_Endln "FILE:%s\nFUNCTION:%s\nLINE:%d", __FILE__, __FUNCTION__, __LINE__);
    while (1);
}

/**
 * OS Panic
 */
void NX_DebugPanic(const char *str)
{
    NX_IRQ_Disable();
    NX_LOG_E("!NX_PANIC!");
    NX_SPIN(str);
}
