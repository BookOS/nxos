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

NX_TEST(ProcessCreate)
{
    NX_Process *process = NX_ProcessCreate(0);
    NX_EXPECT_NOT_NULL(process);
    NX_EXPECT_EQ(NX_ProcessDestroy(process), NX_EOK);
    NX_EXPECT_NE(NX_ProcessDestroy(NX_NULL), NX_EOK);
}

NX_TEST(ProcessExecute)
{
    NX_EXPECT_EQ(NX_ProcessExecute("test", "/test", 0), NX_EOK);
}

NX_TEST_TABLE(Process)
{
    NX_TEST_UNIT(ProcessCreate),
    NX_TEST_UNIT(ProcessExecute),
};

NX_TEST_CASE(Process);

#endif
