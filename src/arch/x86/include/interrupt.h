/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Interrupt 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-3      JasonHu           Init
 */

#ifndef __PLATFORM_INTERRUPT__
#define __PLATFORM_INTERRUPT__

#define EXCEPTION_BASE 0x00
#define EXTERNAL_BASE 0x20
#define SYSCALL_BASE 0x80

#define MAX_EXCEPTION_NR 32

#define ERQ_PAGE_FAULT      14

/* irq number */
#define IRQ_CLOCK           (0)
#define IRQ_KEYBOARD        (1)
#define IRQ_CONNECT         (2)   /* connect to slave */
#define IRQ_SERIAL2         (3)
#define IRQ_SERIAL1         (4)
#define IRQ_PARALLEL2       (5)
#define IRQ_FLOPPY          (6)
#define IRQ_PARALLEL1       (7)

#define IRQ_RTCLOCK         (8)   /* real-time clock */
#define IRQ_REDIRECT        (9)   /* redirect to IRQ2 */
#define IRQ_RESERVED10      (10)
#define IRQ_RESERVED11      (11)
#define IRQ_MOUSE           (12)
#define IRQ_FPU             (13)
#define IRQ_HARDDISK        (14)
#define IRQ_RESERVE         (15)

struct NX_HalTrapFrame
{
    NX_U32 vectorNumber;

    NX_U32 edi;
    NX_U32 esi;
    NX_U32 ebp;
    NX_U32 espDummy; /* espdummy not used, only use a position */
    NX_U32 ebx;
    NX_U32 edx;
    NX_U32 ecx;
    NX_U32 eax;

    NX_U32 gs;
    NX_U32 fs;
    NX_U32 es;
    NX_U32 ds;
    NX_U32 errorCode; /* error code will push into stack if exception has it, or not push 0 by ourself */
    NX_U32 eip;
    NX_U32 cs;
    NX_U32 eflags;
    /* 
     * below will push into stack when from user privilege level enter
     * kernel privilege level (syscall/excption/interrupt) 
     */
    NX_U32 esp;
    NX_U32 ss;
} NX_PACKED;
typedef struct NX_HalTrapFrame NX_HalTrapFrame;

void CPU_InitInterrupt(void);

void CPU_TrapFrameDump(NX_HalTrapFrame *frame);

#endif  /* __PLATFORM_INTERRUPT__ */
