/*
 * STMP37XX specific definitions
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
#ifndef __37XX_PLATFORM_H
#define __37XX_PLATFORM_H

#define	MACHINE_ID	0x6c5
#define NULL ((void *)0)


#define OCRAM_BASE	0x00000000
#define OCRAM_SIZE	0x00008000

#define FLASH_BASE	0x70000000
#define FLASH_SIZE	0x00080000

#define SDRAM_BASE	0x40000000

#define REGS_BASE	0x80000000
#define REGS_SIZE	0x00100000

#define ATAGS_BASE_ADDRESS	(SDRAM_BASE + 0x100)
#define KERNEL_BASE_ADDRESS	(SDRAM_BASE + 0x8000)

/* 
 * STMP 37xx RTC persistent register 1 bit 10 indicates
 * that system is being resumed from suspend mode
 */
#define RTC_BASE_ADDR		0x8005C000
#define PERSISTENT_SLEEP_REG	0x8005C070
#define PERSISTENT_SLEEP_BIT	10

#define SLEEP_STATE_FINGERPRINT	0xdeadbeef
#define FINGERPRINT		0x00		/* fingerprint offset */
#endif /* __37XX_PLATFROM_H */
