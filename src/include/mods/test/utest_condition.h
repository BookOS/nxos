/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Unit test condition
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-2      JasonHu           Init
 */

#ifndef __MODS_UTEST_CONDITION__
#define __MODS_UTEST_CONDITION__

#include <xbook.h>

#ifdef CONFIG_NX_ENABLE_TEST_UTEST

/* assert */
#define __UTestAssertEx(value, msg) NX_UTestAssert(value, __FILE__, __LINE__, __func__, msg, NX_True)

#define __UTestAssertStringEx(a, b, equ, msg) \
        NX_UTestAssertString((const char*)(a), (const char*)(b), equ, \
        __FILE__, __LINE__, __func__, msg, NX_True)

#define __UTestAssertBufEx(a, b, sz, equ, msg) \
        NX_UTestAssertBuf((const char*)(a), (const char*)(b), (sz), equ, \
        __FILE__, __LINE__, __func__, msg, NX_True)

#define NX_ASSERT_TRUE(value)   __UTestAssertEx(value, "(" #value ") is false")
#define NX_ASSERT_FALSE(value)  __UTestAssertEx(!(value), "(" #value ") is true")

#define NX_ASSERT_EQ(a, b)  __UTestAssertEx((a) == (b), "(" #a ") not equal to (" #b ")")
#define NX_ASSERT_NE(a, b)  __UTestAssertEx((a) != (b),  "(" #a ") equal to (" #b ")")

#define NX_ASSERT_LT(a, b)  __UTestAssertEx((a) < (b), "(" #a ") >= (" #b ")")
#define NX_ASSERT_GT(a, b)  __UTestAssertEx((a) > (b), "(" #a ") <= (" #b ")")
#define NX_ASSERT_LE(a, b)  __UTestAssertEx((a) <= (b), "(" #a ") > (" #b ")")
#define NX_ASSERT_GE(a, b)  __UTestAssertEx((a) >= (b), "(" #a ") < (" #b ")")

#define NX_ASSERT_NULL(value)      __UTestAssertEx((const char *)(value) == NX_NULL, "(" #value ") is not null")
#define NX_ASSERT_NOT_NULL(value)  __UTestAssertEx((const char *)(value) != NX_NULL, "(" #value ") is null")

#define NX_ASSERT_STREQ(a, b)  __UTestAssertStringEx((const char*)(a), (const char*)(b), NX_True, "string not equal")
#define NX_ASSERT_STRNE(a, b)  __UTestAssertStringEx((const char*)(a), (const char*)(b), NX_False, "string equal")

#define NX_ASSERT_BUFEQ(a, b, sz)  __UTestAssertBufEx((const char*)(a), (const char*)(b), (sz), NX_True, "buf not equal")
#define NX_ASSERT_BUFNE(a, b, sz)  __UTestAssertBufEx((const char*)(a), (const char*)(b), (sz), NX_False, "buf equal")

#define NX_ASSERT_IN_RANGE(value, min, max)  __UTestAssertEx(((value >= min) && (value <= max)), "(" #value ") not in range("#min","#max")")
#define NX_ASSERT_NI_RANGE(value, min, max) __UTestAssertEx(!((value >= min) && (value <= max)), "(" #value ") in range("#min","#max")")

#define NX_ASSERT_TRUE_EX(valu, msg)   __UTestAssertEx(value, msg) 
#define NX_ASSERT_FALSE_EX(value, msg)  __UTestAssertEx(!(value), msg)

#define NX_ASSERT_EQ_EX(a, b, msg)  __UTestAssertEx((a) == (b), msg)
#define NX_ASSERT_NE_EX(a, b, msg)  __UTestAssertEx((a) != (b), msg)

#define NX_ASSERT_LT_EX(a, b, msg)  __UTestAssertEx((a) < (b), msg)
#define NX_ASSERT_GT_EX(a, b, msg)  __UTestAssertEx((a) > (b), msg)
#define NX_ASSERT_LE_EX(a, b, msg)  __UTestAssertEx((a) <= (b), msg)
#define NX_ASSERT_GE_EX(a, b, msg)  __UTestAssertEx((a) >= (b), msg)

#define NX_ASSERT_NULL_EX(value, msg)      __UTestAssertEx((const char *)(value) == NX_NULL, msg)
#define NX_ASSERT_NOT_NULL_EX(value, msg)  __UTestAssertEx((const char *)(value) != NX_NULL, msg)

#define NX_ASSERT_STREQ_EX(a, b, msg)  __UTestAssertStringEx((const char*)(a), (const char*)(b), NX_True, msg)
#define NX_ASSERT_STRNE_EX(a, b, msg)  __UTestAssertStringEx((const char*)(a), (const char*)(b), NX_False, msg)

#define NX_ASSERT_BUFEQ_EX(a, b, sz, msg)  __UTestAssertBufEx((const char*)(a), (const char*)(b), (sz), NX_True, msg)
#define NX_ASSERT_BUFNE_EX(a, b, sz, msg)  __UTestAssertBufEx((const char*)(a), (const char*)(b), (sz), NX_False, msg)

#define NX_ASSERT_IN_RANGE_EX(value, min, max, msg)  __UTestAssertEx(((value >= min) && (value <= max)), msg) 
#define NX_ASSERT_NI_RANGE_EX(value, min, max, msg) __UTestAssertEx(!((value >= min) && (value <= max)), msg)

/* expect */
#define __UTestExpectEx(value, msg) NX_UTestAssert(value, __FILE__, __LINE__, __func__, msg, NX_False)

#define __UTestExpectStringEx(a, b, equ, msg) \
        NX_UTestAssertString((const char*)(a), (const char*)(b), equ, \
        __FILE__, __LINE__, __func__, msg, NX_False)

#define __UTestExpectBufEx(a, b, sz, equ, msg) \
        NX_UTestAssertBuf((const char*)(a), (const char*)(b), (sz), equ, \
        __FILE__, __LINE__, __func__, msg, NX_False)

#define NX_EXPECT_TRUE(value)   __UTestExpectEx(value, "(" #value ") is false")
#define NX_EXPECT_FALSE(value)  __UTestExpectEx(!(value), "(" #value ") is true")

#define NX_EXPECT_EQ(a, b)  __UTestExpectEx((a) == (b), "(" #a ") not equal to (" #b ")")
#define NX_EXPECT_NE(a, b)  __UTestExpectEx((a) != (b),  "(" #a ") equal to (" #b ")")

#define NX_EXPECT_LT(a, b)  __UTestExpectEx((a) < (b), "(" #a ") >= (" #b ")")
#define NX_EXPECT_GT(a, b)  __UTestExpectEx((a) > (b), "(" #a ") <= (" #b ")")
#define NX_EXPECT_LE(a, b)  __UTestExpectEx((a) <= (b), "(" #a ") > (" #b ")")
#define NX_EXPECT_GE(a, b)  __UTestExpectEx((a) >= (b), "(" #a ") < (" #b ")")

#define NX_EXPECT_NULL(value)      __UTestExpectEx((const char *)(value) == NX_NULL, "(" #value ") is not null")
#define NX_EXPECT_NOT_NULL(value)  __UTestExpectEx((const char *)(value) != NX_NULL, "(" #value ") is null")

#define NX_EXPECT_STREQ(a, b)  __UTestExpectStringEx((const char*)(a), (const char*)(b), NX_True, "string not equal")
#define NX_EXPECT_STRNE(a, b)  __UTestExpectStringEx((const char*)(a), (const char*)(b), NX_False, "string equal")

#define NX_EXPECT_BUFEQ(a, b, sz)  __UTestExpectBufEx((const char*)(a), (const char*)(b), (sz), NX_True, "buf not equal")
#define NX_EXPECT_BUFNE(a, b, sz)  __UTestExpectBufEx((const char*)(a), (const char*)(b), (sz), NX_False, "buf equal")

#define NX_EXPECT_IN_RANGE(value, min, max)  __UTestExpectEx(((value >= min) && (value <= max)), "(" #value ") not in range("#min","#max")")
#define NX_EXPECT_NI_RANGE(value, min, max) __UTestExpectEx(!((value >= min) && (value <= max)), "(" #value ") in range("#min","#max")")

#define NX_EXPECT_TRUE_EX(valu, msg)   __UTestExpectEx(value, msg) 
#define NX_EXPECT_FALSE_EX(value, msg)  __UTestExpectEx(!(value), msg)

#define NX_EXPECT_EQ_EX(a, b, msg)  __UTestExpectEx((a) == (b), msg)
#define NX_EXPECT_NE_EX(a, b, msg)  __UTestExpectEx((a) != (b), msg)

#define NX_EXPECT_LT_EX(a, b, msg)  __UTestExpectEx((a) < (b), msg)
#define NX_EXPECT_GT_EX(a, b, msg)  __UTestExpectEx((a) > (b), msg)
#define NX_EXPECT_LE_EX(a, b, msg)  __UTestExpectEx((a) <= (b), msg)
#define NX_EXPECT_GE_EX(a, b, msg)  __UTestExpectEx((a) >= (b), msg)

#define NX_EXPECT_NULL_EX(value, msg)      __UTestExpectEx((const char *)(value) == NX_NULL, msg)
#define NX_EXPECT_NOT_NULL_EX(value, msg)  __UTestExpectEx((const char *)(value) != NX_NULL, msg)

#define NX_EXPECT_STREQ_EX(a, b, msg)  __UTestExpectStringEx((const char*)(a), (const char*)(b), NX_True, msg)
#define NX_EXPECT_STRNE_EX(a, b, msg)  __UTestExpectStringEx((const char*)(a), (const char*)(b), NX_False, msg)

#define NX_EXPECT_BUFEQ_EX(a, b, sz, msg)  __UTestExpectBufEx((const char*)(a), (const char*)(b), (sz), NX_True, msg)
#define NX_EXPECT_BUFNE_EX(a, b, sz, msg)  __UTestExpectBufEx((const char*)(a), (const char*)(b), (sz), NX_False, msg)

#define NX_EXPECT_IN_RANGE_EX(value, min, max, msg)  __UTestExpectEx(((value >= min) && (value <= max)), msg) 
#define NX_EXPECT_NI_RANGE_EX(value, min, max, msg) __UTestExpectEx(!((value >= min) && (value <= max)), msg)

/* functions */
NX_PUBLIC void NX_UTestAssert(int value, const char *file, int line, const char *func, const char *msg, NX_Bool dieAction);
NX_PUBLIC void NX_UTestAssertString(const char *a, const char *b, NX_Bool equal, const char *file, int line, const char *func, const char *msg, NX_Bool dieAction);
NX_PUBLIC void NX_UTestAssertBuf(const char *a, const char *b, NX_Size sz, NX_Bool equal, const char *file, int line, const char *func, const char *msg, NX_Bool dieAction);

#endif /* CONFIG_NX_ENABLE_TEST_UTEST */

#endif /* __MODS_UTEST_CONDITION__ */
