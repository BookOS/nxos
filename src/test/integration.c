/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Integration test
 *           
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-15     JasonHu           Init
 */

#include <test/integration.h>

#ifdef CONFIG_NX_ENABLE_TEST_INTEGRATION

#include <utils/string.h>
#include <utils/memory.h>
#include <utils/log.h>
#include <xbook/debug.h>
#include <sched/thread.h>
#include <xbook/init_call.h>

NX_PRIVATE NX_Integration *integrationTable = NX_NULL;
NX_PRIVATE NX_Size integrationCount;

NX_IMPORT const NX_Addr __NX_IntegrationTableStart;
NX_IMPORT const NX_Addr __NX_IntegrationTableEnd;

NX_PRIVATE void IntegrationInvoke(void)
{
    integrationTable = (NX_Integration *)&__NX_IntegrationTableStart;
    integrationCount = (NX_Integration *) &__NX_IntegrationTableEnd - integrationTable;
    NX_LOG_I("[==========] Total integrations: %d", integrationCount);
    int integrationIndex;
    NX_Error err;
    NX_Size passedTests = 0; 
    for (integrationIndex = 0; integrationIndex < integrationCount; integrationIndex++)
    {
        NX_LOG_I("[==========] [ integration ] Running (%d/%d) test (%s).", integrationIndex + 1, integrationCount, integrationTable->integrationName);
        err = integrationTable->func();
        if (err == NX_EOK)
        {
            passedTests++;
            NX_LOG_I("[==========] [ integration ] (%d/%d) test ran with state %s .",
                integrationIndex + 1, integrationCount, "success");
        }
        else
        {
            NX_LOG_E("[==========] [ integration ] (%d/%d) test ran with state %s .",
                integrationIndex + 1, integrationCount, "failed");
            break;
        }
        integrationTable++;
    }
    NX_LOG_I("[  FINAL   ] %d integration test finshed. %d/%d are passed. %d/%d are failed.",
        integrationCount, passedTests, integrationCount, integrationCount - passedTests, integrationCount);
}

NX_PRIVATE void IntegrationEntry(void *arg)
{
    /* call integration */
    IntegrationInvoke();
}

NX_PRIVATE void IntegrationInit(void)
{
    NX_Thread *thread = NX_ThreadCreate("Integration", IntegrationEntry, NX_NULL, NX_THREAD_PRIORITY_HIGH);
    NX_ASSERT(thread != NX_NULL);
    NX_ASSERT(NX_ThreadStart(thread) == NX_EOK);
}

NX_INIT_TEST(IntegrationInit);

#endif
