/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: arch registers 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-31     JasonHu           Init
 */

#ifndef __PLATFORM_REGS__
#define __PLATFORM_REGS__

#include <nxos.h>

#define SSTATUS_SPP     (1L << 8)  // Previous mode, 1=Supervisor, 0=User
#define SSTATUS_SPIE    (1L << 5) // Supervisor Previous Interrupt Enable
#define SSTATUS_UPIE    (1L << 4) // User Previous Interrupt Enable
#define SSTATUS_SIE     (1L << 1)  // Supervisor Interrupt Enable
#define SSTATUS_UIE     (1L << 0)  // User Interrupt Enable
#define SSTATUS_FS      (1L << 13)  // Float support
#define SSTATUS_XS      (1L << 14)  // Accelerator support
#define SSTATUS_SUM     (1L << 18)  // Supervisor Access User memroy

#define RISCV_XLEN    64

#define SCAUSE_INTERRUPT        (1UL << (RISCV_XLEN - 1))
#define SCAUSE_S_SOFTWARE_INTR  1
#define SCAUSE_S_TIMER_INTR     5
#define SCAUSE_S_EXTERNAL_INTR  9

#define IRQ_S_SOFT  1
#define IRQ_H_SOFT  2
#define IRQ_M_SOFT  3
#define IRQ_S_TIMER 5
#define IRQ_H_TIMER 6
#define IRQ_M_TIMER 7
#define IRQ_S_EXT   9
#define IRQ_H_EXT   10
#define IRQ_M_EXT   11
#define IRQ_COP     12
#define IRQ_HOST    13

#define SIE_SSIE    (1 << IRQ_S_SOFT)
#define SIE_STIE    (1 << IRQ_S_TIMER)
#define SIE_SEIE    (1 << IRQ_S_EXT)

#define SIP_SSIE    (1 << IRQ_S_SOFT)
#define SIP_STIE    (1 << IRQ_S_TIMER)
#define SIP_SEIE    (1 << IRQ_S_EXT)

#ifndef __ASSEMBLY__
/* csr registers read/write */
#define ReadCSR(reg) ({ NX_U64 __tmp; \
    NX_CASM ("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; })

#define WriteCSR(reg, val) ({ \
    if (__builtin_constant_p(val) && (NX_U64)(val) < 32) \
        NX_CASM ("csrw " #reg ", %0" :: "i"(val)); \
    else \
        NX_CASM ("csrw " #reg ", %0" :: "r"(val)); })

#define SwapCSR(reg, val) ({ NX_U64 __tmp; \
    if (__builtin_constant_p(val) && (NX_U64)(val) < 32) \
        NX_CASM ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "i"(val)); \
    else \
        NX_CASM ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "r"(val)); \
    __tmp; })

#define SetCSR(reg, bit) ({ NX_U64 __tmp; \
    if (__builtin_constant_p(bit) && (NX_U64)(bit) < 32) \
        NX_CASM ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "i"(bit)); \
    else \
        NX_CASM ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "r"(bit)); \
    __tmp; })

#define ClearCSR(reg, bit) ({ NX_U64 __tmp; \
    if (__builtin_constant_p(bit) && (NX_U64)(bit) < 32) \
        NX_CASM ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "i"(bit)); \
    else \
        NX_CASM ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "r"(bit)); \
    __tmp; })

/* read/write memory */
#define Read8(addr) (*(NX_VOLATILE NX_U8 *)(addr))
#define Read16(addr) (*(NX_VOLATILE NX_U16 *)(addr))
#define Read32(addr) (*(NX_VOLATILE NX_U32 *)(addr))
#define Read64(addr) (*(NX_VOLATILE NX_U64 *)(addr))

#define Write8(addr, value)                      \
    {                                        \
        (*(NX_VOLATILE NX_U8 *)(addr)) = (value); \
    }
#define Write16(addr, value)                       \
    {                                         \
        (*(NX_VOLATILE NX_U16 *)(addr)) = (value); \
    }
#define Write32(addr, value)                       \
    {                                         \
        (*(NX_VOLATILE NX_U32 *)(addr)) = (value); \
    }
#define Write64(addr, value)                       \
    {                                         \
        (*(NX_VOLATILE NX_U64 *)(addr)) = (value); \
    }

#define ReadReg(reg) ({ NX_U64 __tmp; \
    NX_CASM ("mv %0, " #reg : "=r"(__tmp)); \
    __tmp; })

#define WriteReg(reg, val) ({ \
        NX_CASM ("mv " #reg ", %0" :: "r"(val)); })

#endif /* !__ASSEMBLY__ */

#endif  /* __PLATFORM_REGS__ */
