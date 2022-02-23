/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Unit test
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-2      JasonHu           Init
 */

#ifndef __MODS_UTEST__
#define __MODS_UTEST__

#include <xbook/init_call.h>
#include <utils/log.h>

#ifdef CONFIG_NX_ENABLE_TEST_UTEST

#include <mods/test/utest_condition.h>

struct NX_UTest
{
    char *testName;
    void (*func)(void);
    void (*setup)(void);
    void (*clean)(void);
};
typedef struct NX_UTest NX_UTest;

struct NX_UTestCase
{
    char *caseName;
    NX_Error (*setup)(void);
    NX_Error (*clean)(void);
    NX_UTest *unitTable;
    NX_Size unitCount;
};
typedef struct NX_UTestCase NX_UTestCase;

struct NX_UTestSum
{
    NX_Bool hasError;
    NX_U32 passedNum;
    NX_U32 failedNum;
};
typedef struct NX_UTestSum NX_UTestSum;

#define NX_TEST(name) \
    NX_PRIVATE void Utest##name(void)

#define NX_TEST_SETUP(name) \
    NX_PRIVATE void Utest##name##Setup(void)

#define NX_TEST_CLEAN(name) \
    NX_PRIVATE void Utest##name##Clean(void)

#define NX_TEST_TABLE(name) \
    NX_USED NX_PRIVATE NX_UTest __UTestCase##name##Table[] =  

#define NX_TEST_UNIT(name) \
    { \
        #name, \
        Utest##name, \
        NX_NULL, \
        NX_NULL \
    }

#define NX_TEST_UNIT_SETUP_CLEAN(name) \
    { \
        #name, \
        Utest##name, \
        Utest##name##Setup, \
        Utest##name##Clean \
    }

#define NX_TEST_UNIT_SETUP(name) \
    { \
        #name, \
        Utest##name, \
        Utest##name##Setup, \
        NX_NULL \
    }

#define NX_TEST_UNIT_CLEAN(name) \
    { \
        #name, \
        Utest##name, \
        NX_NULL, \
        Utest##name##Clean \
    }

#define NX_TEST_CASE_SETUP(name)  NX_Error __UTestCase##name##Setup(void)
#define NX_TEST_CASE_CLEAN(name)  NX_Error __UTestCase##name##Clean(void)

#define NX_TEST_CASE(name) \
    NX_TEST_CASE_SETUP(name) NX_WEAK_SYM;\
    NX_TEST_CASE_CLEAN(name) NX_WEAK_SYM;\
    NX_USED NX_PRIVATE const NX_UTestCase __utestTestcase \
    NX_SECTION("UTestCaseTable") = \
    { \
        .caseName = #name, \
        .setup = __UTestCase##name##Setup, \
        .clean = __UTestCase##name##Clean, \
        .unitTable = __UTestCase##name##Table, \
        .unitCount = NX_ARRAY_SIZE(__UTestCase##name##Table) \
    }

#endif /* CONFIG_NX_ENABLE_TEST_UTEST */

#endif  /* __MODS_UTEST__ */
