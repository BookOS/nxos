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

#include <mods/test/utest.h>

#ifdef CONFIG_NX_ENABLE_TEST_UTEST

#include <utils/string.h>
#include <utils/memory.h>
#include <xbook/debug.h>
#include <sched/thread.h>
#include <xbook/init_call.h>

NX_PRIVATE NX_UTestCase *TestCaseTable = NX_NULL;
NX_PRIVATE NX_Size TestCaseCount;
NX_PRIVATE NX_UTestSum LocalUtestSum = {NX_False, 0, 0};
NX_PRIVATE NX_UTestSum UtestSum = {NX_False, 0, 0};
NX_PRIVATE NX_UTestSum UtestCaseSum = {NX_False, 0, 0};

NX_IMPORT const NX_Addr __NX_UTestCaseTableStart;
NX_IMPORT const NX_Addr __NX_UTestCaseTableEnd;

NX_PRIVATE void NX_UTestInvoke(void)
{
    UtestCaseSum.hasError = NX_False;
    UtestCaseSum.passedNum = 0;
    UtestCaseSum.failedNum = 0;

    TestCaseTable = (NX_UTestCase *)&__NX_UTestCaseTableStart;
    TestCaseCount = (NX_UTestCase *) &__NX_UTestCaseTableEnd - TestCaseTable;
    NX_LOG_I("[==========] Total test case: %d", TestCaseCount);
    int testCaseIndex = 0;
    int testIndex = 0;
    for (testCaseIndex = 0; testCaseIndex < TestCaseCount; testCaseIndex++)
    {
        NX_LOG_I("[==========] [ testcase ] Running %d tests from test case (%s).", TestCaseTable->unitCount, TestCaseTable->caseName);
        if (TestCaseTable->setup != NX_NULL)
        {
            NX_LOG_I("[----------] [ testcase ] Global test (%s) set-up.", TestCaseTable->caseName);
            if (TestCaseTable->setup() != NX_EOK)
            {
                NX_LOG_E("[  FAILED  ] [ testcase ] Global test (%s) set-up.", TestCaseTable->caseName);
                UtestCaseSum.failedNum++;
                goto __TestCaseContinue;
            }
        }

        if (TestCaseTable->unitTable != NX_NULL)
        {
            UtestSum.hasError = NX_False;
            UtestSum.passedNum = 0;
            UtestSum.failedNum = 0;
            for (testIndex = 0; testIndex < TestCaseTable->unitCount; testIndex++)
            {
                NX_UTest *utest = (NX_UTest *)&TestCaseTable->unitTable[testIndex];

                if (utest->setup != NX_NULL)
                {
                    NX_LOG_I("[----------] [   test   ] Local test (%s.%s) set-up.", TestCaseTable->caseName, utest->testName);
                    utest->setup();
                }
                if (utest->func != NX_NULL)
                {
                    NX_LOG_I("[ RUN      ] [   test   ] %s.%s", TestCaseTable->caseName, utest->testName);
                    LocalUtestSum.hasError = NX_False;
                    LocalUtestSum.passedNum = 0;
                    LocalUtestSum.failedNum = 0;
                    utest->func();
                    if (LocalUtestSum.failedNum == 0)
                    {
                        NX_LOG_I("[  PASSED  ] [   test   ] %s.%s", TestCaseTable->caseName, utest->testName);
                        UtestSum.passedNum++;
                    }
                    else
                    {
                        NX_LOG_E("[  FAILED  ] [   test   ] %s.%s", TestCaseTable->caseName, utest->testName);
                        UtestSum.failedNum++;
                    }
                    NX_LOG_I("[   SUM    ] [   test   ] test finshed. %d are passed. %d are failed.", 
                        LocalUtestSum.passedNum, LocalUtestSum.failedNum);
                }
                else
                {            
                    NX_LOG_E("[  FAILED  ] [   test   ] %s.%s", TestCaseTable->caseName, utest->testName);
                }

                if (utest->clean != NX_NULL)
                {
                    NX_LOG_I("[----------] [   test   ] Local test (%s.%s) tear-down.", TestCaseTable->caseName, utest->testName);
                    utest->clean();
                }
            }
            if (UtestSum.failedNum == 0)
            {
                UtestCaseSum.passedNum++;
            }
            else
            {
                UtestCaseSum.failedNum++;
            }
            NX_LOG_I("[   SUM    ] [ testcase ] %d tests finshed. %d/%d are passed. %d/%d are failed.", 
                TestCaseTable->unitCount, UtestSum.passedNum, TestCaseTable->unitCount, 
                UtestSum.failedNum, TestCaseTable->unitCount);

        }
        else
        {
            NX_LOG_E("[  FAILED  ] [ testcase ] %s", TestCaseTable->caseName);
        }

        if (TestCaseTable->clean != NX_NULL)
        {
            NX_LOG_I("[----------] [ testcase ] Global test (%s) tear-down.", TestCaseTable->caseName);
            if (TestCaseTable->clean() != NX_EOK)
            {
                NX_LOG_E("[  FAILED  ] [ testcase ] Global test (%s) tear-down.", TestCaseTable->caseName);
                UtestCaseSum.failedNum++;
                goto __TestCaseContinue;
            }
        }
__TestCaseContinue:
        NX_LOG_I("[==========] [ testcase ] %d tests from test case (%s) ran.",
            testIndex > 0 ? testIndex + 1 : 0, TestCaseTable->caseName);
        TestCaseTable++;
    }
    NX_LOG_I("[  FINAL   ] %d test cases finshed. %d/%d are passed. %d/%d are failed.",
        TestCaseCount, UtestCaseSum.passedNum, TestCaseCount, UtestCaseSum.failedNum, TestCaseCount);

}

void NX_UTestAssert(int value, const char *file, int line, const char *func, const char *msg, NX_Bool dieAction)
{
    if (value)
    {
        LocalUtestSum.hasError = NX_False;
        LocalUtestSum.passedNum++;
        NX_LOG_I("[       OK ] [   point  ] %s:%d", func, line);
    }
    else
    {
        LocalUtestSum.hasError = NX_True;
        LocalUtestSum.failedNum++;
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
    NX_ASSERT(NX_ThreadRun(thread) == NX_EOK);
}

NX_INIT_TEST(NX_UTestInit);

#endif
