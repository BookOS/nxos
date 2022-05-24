/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Platfrom header
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-18     JasonHu           Init
 */

#ifndef __PLATFORM_HEADER__
#define __PLATFORM_HEADER__

#include <nxos.h>

#define DRAM_SIZE_DEFAULT (256 * NX_MB)

#define MEM_SBI_BASE    0x80000000UL
#define MEM_SBI_SZ      (2 * NX_MB)

#define MEM_KERNEL_BASE (MEM_SBI_BASE + MEM_SBI_SZ)
#define MEM_KERNEL_SZ   (30 * NX_MB)

#define MEM_NORMAL_BASE (MEM_KERNEL_BASE + MEM_KERNEL_SZ)

#define MEM_MIN_SIZE    (128 * NX_MB)

#define MEM_KERNEL_SPACE_SZ  (128 * NX_MB)

#define MEM_KERNEL_TOP  (MEM_SBI_BASE + MEM_KERNEL_SPACE_SZ)

/* max cpus for hifive unmached (1: u540, 4: u740) */
#define PLATFORM_MAX_NR_MULTI_CORES 5

/**
 * Physical memory layout:
 *
 * +------------------------+ <- MAX PHY SIZE (TOP to 4GB)
 * | L3 PAGES               |
 * +------------------------+ <- KERNEL PAGES (TOP to 2GB + 128MB)
 * | L3(K) & L0/1/2  PAGES  |
 * +------------------------+ <- 0x82000000 (2GB + 32MB)
 * | KERNEL                 |
 * +------------------------+ <- 0x80200000 (2GB + 2MB)
 * | OPENSBI                |
 * +------------------------+ <- 0x80000000 (2GB)
 * | MMIO/UNMAPPED          |
 * +------------------------+ <- 0x00000000
 */

/**
 * Virtual memory layout:
 * 
 * @: delay map
 * 
 * +------------------------+ <- 0x400000000 (16GB)
 * | @USER                  | 
 * +------------------------+ <- 0xFFFFFFFF (4GB)
 * | @RESERVED              |
 * +------------------------+ <- 0x88000000 (2GB + 128MB)
 * | @KMAP                  |
 * +------------------------+ <- 0x84000000 (2GB + 64MB)
 * | KHEAP                  |
 * +------------------------+ <- 0x82000000 (2GB + 32MB)
 * | KCODE & KDATA & KBSS   |
 * +------------------------+ <- 0x80200000 (2GB + 2MB)
 * | OPENSBI                |
 * +------------------------+ <- 0x80000000 (2GB)
 * | MMIO/UNMAPPED          |
 * +------------------------+ <- 0x00000000
 */

NX_IMPORT void MMU_EarlyMap(void);
void *NX_HalGetKernelPageTable(void);

#endif /* __PLATFORM_HEADER__ */
