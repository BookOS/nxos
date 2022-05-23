/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Platfrom header
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-4-17      JasonHu           Init
 */

#ifndef __PLATFORM_HEADER__
#define __PLATFORM_HEADER__

#include <nxos.h>

#define DRAM_SIZE_DEFAULT (512 * NX_MB)

#define MEM_SBI_BASE    0x40000000UL
#define MEM_SBI_SZ      (2 * NX_MB)

#define MEM_KERNEL_BASE (MEM_SBI_BASE + MEM_SBI_SZ)
#define MEM_KERNEL_SZ   (30 * NX_MB)

#define MEM_NORMAL_BASE (MEM_KERNEL_BASE + MEM_KERNEL_SZ)

#define MEM_MIN_SIZE    (128 * NX_MB)

#define MEM_KERNEL_SPACE_SZ  (128 * NX_MB)

#define MEM_KERNEL_TOP  (MEM_SBI_BASE + MEM_KERNEL_SPACE_SZ)

/* max cpus for qemu */
#define PLATFORM_MAX_NR_MULTI_CORES 1

/**
 * Physical memory layout:
 *
 * +------------------------+ <- MAX PHY SIZE (TOP to 3GB)
 * | AVALIABLE PAGES        |
 * +------------------------+ <- 0x42000000 (1GB + 32MB)
 * | KERNEL                 |
 * +------------------------+ <- 0x40200000 (1GB + 2MB)
 * | OPENSBI                |
 * +------------------------+ <- 0x40000000 (1GB)
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
 * +------------------------+ <- 0x42000000 (1GB + 32MB)
 * | KCODE & KDATA & KBSS   |
 * +------------------------+ <- 0x40200000 (1GB + 2MB)
 * | OPENSBI                |
 * +------------------------+ <- 0x40000000 (1GB)
 * | MMIO/UNMAPPED          |
 * +------------------------+ <- 0x00000000
 */

NX_IMPORT void MMU_EarlyMap(void);
void *NX_HalGetKernelPageTable(void);

#endif /* __PLATFORM_HEADER__ */
