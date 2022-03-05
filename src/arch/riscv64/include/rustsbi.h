/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Rust Supervisor Binary Interface(SBI)
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-1-26      JasonHu           Init
 */

/*
 * Copyright (C) 2015 Regents of the University of California
 */

#ifndef __RISCV_RUST_SBI__
#define __RISCV_RUST_SBI__

#include <xbook.h>

#define SBI_SET_TIMER 0
#define SBI_CONSOLE_PUTCHAR 1
#define SBI_CONSOLE_GETCHAR 2
#define SBI_CLEAR_IPI 3
#define SBI_SEND_IPI 4
#define SBI_REMOTE_FENCE_I 5
#define SBI_REMOTE_SFENCE_VMA 6
#define SBI_REMOTE_SFENCE_VMA_ASID 7
#define SBI_SHUTDOWN 8

#define SBI_CALL(which, arg0, arg1, arg2, arg3) ({		\
	register NX_Addr a0 asm ("a0") = (NX_Addr)(arg0);	\
	register NX_Addr a1 asm ("a1") = (NX_Addr)(arg1);	\
	register NX_Addr a2 asm ("a2") = (NX_Addr)(arg2);	\
	register NX_Addr a3 asm ("a3") = (NX_Addr)(arg3);	\
	register NX_Addr a7 asm ("a7") = (NX_Addr)(which);	\
	NX_CASM ("ecall"					\
		      : "+r" (a0)				\
		      : "r" (a1), "r" (a2), "r" (a3), "r" (a7)	\
		      : "memory");				\
	a0;							\
})

/* Lazy implementations until SBI is finalized */
#define SBI_CALL_0(which) SBI_CALL(which, 0, 0, 0, 0)
#define SBI_CALL_1(which, arg0) SBI_CALL(which, arg0, 0, 0, 0)
#define SBI_CALL_2(which, arg0, arg1) SBI_CALL(which, arg0, arg1, 0, 0)
#define SBI_CALL_3(which, arg0, arg1, arg2) \
		SBI_CALL(which, arg0, arg1, arg2, 0)
#define SBI_CALL_4(which, arg0, arg1, arg2, arg3) \
		SBI_CALL(which, arg0, arg1, arg2, arg3)

NX_INLINE void sbi_console_putchar(int ch)
{
	SBI_CALL_1(SBI_CONSOLE_PUTCHAR, ch);
}

NX_INLINE int sbi_console_getchar(void)
{
	return SBI_CALL_0(SBI_CONSOLE_GETCHAR);
}

NX_INLINE void sbi_set_timer(NX_Size stime_value)
{
	SBI_CALL_1(SBI_SET_TIMER, stime_value);
}

NX_INLINE void sbi_shutdown(void)
{
	SBI_CALL_0(SBI_SHUTDOWN);
}

NX_INLINE void sbi_clear_ipi(void)
{
	SBI_CALL_0(SBI_CLEAR_IPI);
}

NX_INLINE void sbi_send_ipi(const NX_UArch *hart_mask)
{
	SBI_CALL_1(SBI_SEND_IPI, hart_mask);
}

NX_INLINE void sbi_remote_fence_i(const NX_UArch *hart_mask)
{
	SBI_CALL_1(SBI_REMOTE_FENCE_I, hart_mask);
}

NX_INLINE void sbi_remote_sfence_vma(const NX_UArch *hart_mask,
									 NX_UArch start,
									 NX_UArch size)
{
	SBI_CALL_3(SBI_REMOTE_SFENCE_VMA, hart_mask, start, size);
}

NX_INLINE void sbi_remote_sfence_vma_asid(const NX_UArch *hart_mask,
										  NX_UArch start,
										  NX_UArch size,
										  NX_UArch asid)
{
	SBI_CALL_4(SBI_REMOTE_SFENCE_VMA_ASID, hart_mask, start, size, asid);
}

NX_INLINE void sbi_set_extern_interrupt(NX_UArch func_pointer) {
	NX_CASM("mv a6, %0" : : "r" (0x210));
	SBI_CALL_1(0x0A000004, func_pointer);
}

NX_INLINE void sbi_set_mie(void) {
	SBI_CALL_0(0x0A000005);
}

#endif /* __RISCV_RUST_SBI__ */
