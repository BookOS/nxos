/**
 * Copyright (c) 2018-2021, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Configure for each platform
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-9-17      JasonHu           Init
 */

#ifndef __OS_CONFIG__
#define __OS_CONFIG__

/* OS normal config */
#define CONFIG_KERNEL_VADDR_START 0
#define CONFIG_TICKS_PER_SECOND 100
#define CONFIG_IRQ_NAME_LEN 48

/* kernel virtual start addr */
#define CONFIG_KERNEL_VSTART 0x0000000

/* OS debug config */
#define CONFIG_ASSERT_DEBUG

/* OS test config */
#define CONFIG_PLATFROM_TEST
#ifdef CONFIG_PLATFROM_TEST
    /* #define CONFIG_THREAD_TEST */
    #define CONFIG_CONSOLE_TEST
    // #define CONFIG_PAGE_TEST
#endif

#endif  /* __OS_CONFIG__ */