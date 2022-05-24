/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: NX_Thread ID test
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-16     JasonHu           Init
 */

#include <base/thread_id.h>
#include <base/log.h>
#include <test/integration.h>

#ifdef CONFIG_NX_TEST_INTEGRATION_THREAD_ID

NX_INTEGRATION_TEST(TestThreadID)
{
    int i;
    for (i = 0; i < 32; i++)
    {
        int id = NX_ThreadIdAlloc();
        NX_LOG_D("alloc id: %d", id);
        NX_ThreadIdFree(id);
    }
    
    for (i = 0; i < 4; i++)
    {
        int id = NX_ThreadIdAlloc();
        NX_LOG_D("alloc id: %d", id);
        NX_ThreadIdFree(id);
    }
    return NX_EOK;
}

#endif
