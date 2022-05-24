/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Integration test
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-16     JasonHu           Init
 */

#ifndef __MODS_TEST_INTEGRATION__
#define __MODS_TEST_INTEGRATION__

#include <nxos.h>

#ifdef CONFIG_NX_ENABLE_TEST_INTEGRATION

struct NX_Integration
{
    char *integrationName;
    NX_Error (*func)(void);
};
typedef struct NX_Integration NX_Integration;

#define __INTEGRATION_TEST(name)  NX_Error __Integration##name##Test(void)

#define NX_INTEGRATION_TEST(name) \
    NX_PRIVATE __INTEGRATION_TEST(name); \
    NX_USED NX_PRIVATE const NX_Integration __integrationTest \
    NX_SECTION("IntegrationTable") = \
    { \
        .integrationName = #name, \
        .func = __Integration##name##Test, \
    }; \
    NX_PRIVATE __INTEGRATION_TEST(name) \

#endif /* CONFIG_NX_ENABLE_TEST_INTEGRATION */

#endif /* __MODS_TEST_INTEGRATION__ */

