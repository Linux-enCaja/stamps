/*
 * linux_prep entry code.
 *
 * Entered twice:
 *
 * - Before loading Linux kernel image to SDRAM to check
 *   if CPU is being taken out of standby mode and resume
 *   sleeping kernel.
 * 
 * - After loading Linux kernel image to prepare Linux boot
 *   paramteters and jump to the kernel
 *
 * Vladislav Buzov <vbuzov@embeddedalley.com>
 *
 * Copyright 2008 SigmaTel, Inc
 * Copyright 2008 Embedded Alley Solutions, Inc
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2.  This program  is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */
#include "platform.h"

#undef DEBUG

    .code 32
    .section ".start", "ax"

    .global _start, _fail_reason
_start:
    stmdb   sp!, {r4-r12, lr}

    b  start_kernel

	/* Check entry counter */
	ldr	r4, entry_count
	cmp	r4, #0
	bne	start_kernel

#ifdef DEBUG
	mov	r2, #0x80000000
	orr	r2, r2, #0x70000
	str	r0, [r2]
#endif
	/* Update counter to show we were here */
	add	r4, r4, #1
	str	r4, entry_count

	/* Return to ROM */
	ldmia	sp!, {r4-r12, lr}
	bx	lr

	.pool

entry_count:
	.long	0x00
_fail_reason:
	.long 	0x00

start_kernel:
	bl	clear_bss



    /* Enter the Chumby boot shell */
    bl enter_shell



	.pool
/*
 * This function clears out the .bss section.
 */
clear_bss:
	ldr	r0, =_bss_start
	ldr	r1, =_bss_end
	mov	r2, #0
1:	str	r2, [r0], #4
	cmp	r0, r1
	blo	1b
	mov	pc, lr

	.pool
/*
 * ARM EABI toolchain needs divide by zero handler to be implemented
 * externally.
 */
	.globl	__div0
__div0:
	mov	pc, lr
    .globl  raise
raise:
	mov	pc, lr
