/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: process utest 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-13      JasonHu           Init
 */

#include <test/utest.h>
#include <process/process.h>

#ifdef CONFIG_NX_UTEST_SCHED_PROCESS

NX_TEST(ProcessExecute)
{
    NX_EXPECT_EQ(NX_ProcessLaunch("test", "/test", 0), NX_EOK);
}

NX_TEST_TABLE(Process)
{
    NX_TEST_UNIT(ProcessExecute),
};

NX_TEST_CASE(Process);

#endif
