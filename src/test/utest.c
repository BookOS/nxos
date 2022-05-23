/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Unit test
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-2      JasonHu           Init
 */

#include <test/utest.h>

#ifdef CONFIG_NX_ENABLE_TEST_UTEST

#include <utils/string.h>
#include <utils/memory.h>
#include <xbook/debug.h>
#include <sched/thread.h>
#include <xbook/init_call.h>

NX_PRIVATE NX_UTestCase *testCaseTable = NX_NULL;
NX_PRIVATE NX_Size testCaseCount;
NX_PRIVATE NX_UTestSum localUtestSum = {NX_False, 0, 0};
NX_PRIVATE NX_UTestSum utestSum = {NX_False, 0, 0};
NX_PRIVATE NX_UTestSum utestCaseSum = {NX_False, 0, 0};

NX_IMPORT const NX_Addr __NX_UTestCaseTableStart;
NX_IMPORT const NX_Addr __NX_UTestCaseTableEnd;

NX_PRIVATE void NX_UTestInvoke(void)
{
    utestCaseSum.hasError = NX_False;
    utestCaseSum.passedNum = 0;
    utestCaseSum.failedNum = 0;

    testCaseTable = (NX_UTestCase *)&__NX_UTestCaseTableStart;
    testCaseCount = (NX_UTestCase *) &__NX_UTestCaseTableEnd - testCaseTable;
    NX_LOG_I("[==========] Total test case: %d", testCaseCount);
    int testCaseIndex = 0;
    int testIndex = 0;
    for (testCaseIndex = 0; testCaseIndex < testCaseCount; testCaseIndex++)
    {
        NX_LOG_I("[==========] [ testcase ] Running %d tests from test case (%s).", testCaseTable->unitCount, testCaseTable->caseName);
        if (testCaseTable->setup != NX_NULL)
        {
            NX_LOG_I("[----------] [ testcase ] Global test (%s) set-up.", testCaseTable->caseName);
            if (testCaseTable->setup() != NX_EOK)
            {
                NX_LOG_E("[  FAILED  ] [ testcase ] Global test (%s) set-up.", testCaseTable->caseName);
                utestCaseSum.failedNum++;
                goto __TestCaseContinue;
            }
        }

        if (testCaseTable->unitTable != NX_NULL)
        {
            utestSum.hasError = NX_False;
            utestSum.passedNum = 0;
            utestSum.failedNum = 0;
            for (testIndex = 0; testIndex < testCaseTable->unitCount; testIndex++)
            {
                NX_UTest *utest = (NX_UTest *)&testCaseTable->unitTable[testIndex];

                if (utest->setup != NX_NULL)
                {
                    NX_LOG_I("[----------] [   test   ] Local test (%s.%s) set-up.", testCaseTable->caseName, utest->testName);
                    utest->setup();
                }
                if (utest->func != NX_NULL)
                {
                    NX_LOG_I("[ RUN      ] [   test   ] %s.%s", testCaseTable->caseName, utest->testName);
                    localUtestSum.hasError = NX_False;
                    localUtestSum.passedNum = 0;
                    localUtestSum.failedNum = 0;
                    utest->func();
                    if (localUtestSum.failedNum == 0)
                    {
                        NX_LOG_I("[  PASSED  ] [   test   ] %s.%s", testCaseTable->caseName, utest->testName);
                        utestSum.passedNum++;
                    }
                    else
                    {
                        NX_LOG_E("[  FAILED  ] [   test   ] %s.%s", testCaseTable->caseName, utest->testName);
                        utestSum.failedNum++;
                    }
                    NX_LOG_I("[   SUM    ] [   test   ] test finshed. %d are passed. %d are failed.", 
                        localUtestSum.passedNum, localUtestSum.failedNum);
                }
                else
                {            
                    NX_LOG_E("[  FAILED  ] [   test   ] %s.%s", testCaseTable->caseName, utest->testName);
                }

                if (utest->clean != NX_NULL)
                {
                    NX_LOG_I("[----------] [   test   ] Local test (%s.%s) tear-down.", testCaseTable->caseName, utest->testName);
                    utest->clean();
                }
            }
            if (utestSum.failedNum == 0)
            {
                utestCaseSum.passedNum++;
            }
            else
            {
                utestCaseSum.failedNum++;
            }
            NX_LOG_I("[   SUM    ] [ testcase ] %d tests finshed. %d/%d are passed. %d/%d are failed.", 
                testCaseTable->unitCount, utestSum.passedNum, testCaseTable->unitCount, 
                utestSum.failedNum, testCaseTable->unitCount);

        }
        else
        {
            NX_LOG_E("[  FAILED  ] [ testcase ] %s", testCaseTable->caseName);
        }

        if (testCaseTable->clean != NX_NULL)
        {
            NX_LOG_I("[----------] [ testcase ] Global test (%s) tear-down.", testCaseTable->caseName);
            if (testCaseTable->clean() != NX_EOK)
            {
                NX_LOG_E("[  FAILED  ] [ testcase ] Global test (%s) tear-down.", testCaseTable->caseName);
                utestCaseSum.failedNum++;
                goto __TestCaseContinue;
            }
        }
__TestCaseContinue:
        NX_LOG_I("[==========] [ testcase ] %d tests from test case (%s) ran.",
            testIndex > 0 ? testIndex + 1 : 0, testCaseTable->caseName);
        testCaseTable++;
    }
    NX_LOG_I("[  FINAL   ] %d test cases finshed. %d/%d are passed. %d/%d are failed.",
        testCaseCount, utestCaseSum.passedNum, testCaseCount, utestCaseSum.failedNum, testCaseCount);

}

void NX_UTestAssert(int value, const char *file, int line, const char *func, const char *msg, NX_Bool dieAction)
{
    if (value)
    {
        localUtestSum.hasError = NX_False;
        localUtestSum.passedNum++;
        NX_LOG_I("[       OK ] [   point  ] %s:%d", func, line);
    }
    else
    {
        localUtestSum.hasError = NX_True;
        localUtestSum.failedNum++;
        NX_LOG_E("[  FAILED  ] [   point  ] Failure at:%s Line:%d Message:%s", file, line, msg);
        if (dieAction)
        {
            /* TODO: exit thread? */
            NX_PANIC("Assert!");
        }
    }
}

void NX_UTestAssertString(const char *a, const char *b, NX_Bool equal, 
    const char *file, int line, const char *func, const char *msg, NX_Bool dieAction)
{
    if (a == NX_NULL || b == NX_NULL)
    {
        NX_UTestAssert(0, file, line, func, msg, dieAction);
    }

    if (equal)
    {
        if (NX_StrCmp(a, b) == 0)
        {
            NX_UTestAssert(1, file, line, func, msg, dieAction);
        }
        else
        {
            NX_UTestAssert(0, file, line, func, msg, dieAction);
        }
    }
    else
    {
        if (NX_StrCmp(a, b) == 0)
        {
            NX_UTestAssert(0, file, line, func, msg, dieAction);
        }
        else
        {
            NX_UTestAssert(1, file, line, func, msg, dieAction);
        }
    }
}

void NX_UTestAssertBuf(const char *a, const char *b, NX_Size sz, NX_Bool equal,
    const char *file, int line, const char *func, const char *msg, NX_Bool dieAction)
{
    if (a == NX_NULL || b == NX_NULL)
    {
        NX_UTestAssert(0, file, line, func, msg, dieAction);
    }

    if (equal)
    {
        if (NX_CompareN(a, b, sz) == 0)
        {
            NX_UTestAssert(1, file, line, func, msg, dieAction);
        }
        else
        {
            NX_UTestAssert(0, file, line, func, msg, dieAction);
        }
    }
    else
    {
        if (NX_CompareN(a, b, sz) == 0)
        {
            NX_UTestAssert(0, file, line, func, msg, dieAction);
        }
        else
        {
            NX_UTestAssert(1, file, line, func, msg, dieAction);
        }
    }
}

NX_PRIVATE void NX_UTestEntry(void *arg)
{
    /* call utest */
    NX_UTestInvoke();
}

NX_PRIVATE void NX_UTestInit(void)
{
    NX_Thread *thread = NX_ThreadCreate("UTest", NX_UTestEntry, NX_NULL, NX_THREAD_PRIORITY_HIGH);
    NX_ASSERT(thread != NX_NULL);
    NX_ASSERT(NX_ThreadStart(thread) == NX_EOK);
}

NX_INIT_TEST(NX_UTestInit);

#endif
