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

    .global _start
_start:

// Save our load address as the global base_address.
    mov r0, pc
    sub r0, r0, #8
    sub r0, r0, #0x2000
    ldr r1, =base_address
    str r0, [r1]


	bl	clear_bss

    /* Enter the Chumby boot shell */
    bl enter_shell

    .pool
.global base_address
base_address:
    .long 0xc111b111


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
