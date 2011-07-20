/* 
 * Linux parameters setting code
 *
 * - Parse a command line linux_prep section
 *
 * - Detect a magic key combination to choice a proper
 *   command line
 *
 * - Generate a list of ARM tags handled by Linux kernel
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
#include <setup.h>
#include <stmp3xxx.h>
#include <arch/platform.h>
#include "serial.h"


#define USB_BACKLIGHT_HACK

#define REGS_PXP_BASE_PHYS (0x8002A000)
#define HW_PXP_CTRL     REGS_PXP_BASE_PHYS+0x000
#define HW_PXP_CTRL_SET REGS_PXP_BASE_PHYS+0x004
#define HW_PXP_CTRL_CLR REGS_PXP_BASE_PHYS+0x004
#define HW_PXP_CTRL_TOG REGS_PXP_BASE_PHYS+0x004


#define REGS_LCDIF_BASE (0x80030000)
#define HW_LCDIF_CTRL_ADDR (REGS_LCDIF_BASE + 0x00000000)
#define HW_LCDIF_CTRL_SET(x) ((*(u32 *)(HW_LCDIF_CTRL_ADDR+4))=x)
#define HW_LCDIF_CTRL_CLR(x) ((*(u32 *)(HW_LCDIF_CTRL_ADDR+8))=x)
#define BM_LCDIF_CTRL_SFTRST 0x80000000
#define BM_LCDIF_CTRL_CLKGATE 0x40000000
#define BM_LCDIF_CTRL_YCBCR422_INPUT 0x20000000
#define BM_LCDIF_CTRL_WAIT_FOR_VSYNC_EDGE 0x08000000
#define BM_LCDIF_CTRL_DATA_SHIFT_DIR 0x04000000
#define BV_LCDIF_CTRL_DATA_SHIFT_DIR__TXDATA_SHIFT_LEFT  0x0
#define BV_LCDIF_CTRL_DATA_SHIFT_DIR__TXDATA_SHIFT_RIGHT 0x1
#define BP_LCDIF_CTRL_SHIFT_NUM_BITS      21
#define BM_LCDIF_CTRL_SHIFT_NUM_BITS 0x03E00000
#define BF_LCDIF_CTRL_SHIFT_NUM_BITS(v)  \
    (((v) << 21) & BM_LCDIF_CTRL_SHIFT_NUM_BITS)
#define BM_LCDIF_CTRL_DVI_MODE 0x00100000
#define BM_LCDIF_CTRL_BYPASS_COUNT 0x00080000
#define BM_LCDIF_CTRL_VSYNC_MODE 0x00040000
#define BM_LCDIF_CTRL_DOTCLK_MODE 0x00020000
#define BM_LCDIF_CTRL_DATA_SELECT 0x00010000
#define BV_LCDIF_CTRL_DATA_SELECT__CMD_MODE  0x0
#define BV_LCDIF_CTRL_DATA_SELECT__DATA_MODE 0x1
#define BP_LCDIF_CTRL_INPUT_DATA_SWIZZLE      14
#define BM_LCDIF_CTRL_INPUT_DATA_SWIZZLE 0x0000C000
#define BF_LCDIF_CTRL_INPUT_DATA_SWIZZLE(v)  \
    (((v) << 14) & BM_LCDIF_CTRL_INPUT_DATA_SWIZZLE)
#define BV_LCDIF_CTRL_INPUT_DATA_SWIZZLE__NO_SWAP    0x0
#define BV_LCDIF_CTRL_INPUT_DATA_SWIZZLE__LITTLE_ENDIAN   0x0
#define BV_LCDIF_CTRL_INPUT_DATA_SWIZZLE__BIG_ENDIAN_SWAP 0x1
#define BV_LCDIF_CTRL_INPUT_DATA_SWIZZLE__SWAP_ALL_BYTES  0x1
#define BV_LCDIF_CTRL_INPUT_DATA_SWIZZLE__HWD_SWAP  0x2
#define BV_LCDIF_CTRL_INPUT_DATA_SWIZZLE__HWD_BYTE_SWAP   0x3
#define BP_LCDIF_CTRL_CSC_DATA_SWIZZLE      12
#define BM_LCDIF_CTRL_CSC_DATA_SWIZZLE 0x00003000
#define BF_LCDIF_CTRL_CSC_DATA_SWIZZLE(v)  \
    (((v) << 12) & BM_LCDIF_CTRL_CSC_DATA_SWIZZLE)
#define BV_LCDIF_CTRL_CSC_DATA_SWIZZLE__NO_SWAP  0x0
#define BV_LCDIF_CTRL_CSC_DATA_SWIZZLE__LITTLE_ENDIAN   0x0
#define BV_LCDIF_CTRL_CSC_DATA_SWIZZLE__BIG_ENDIAN_SWAP 0x1
#define BV_LCDIF_CTRL_CSC_DATA_SWIZZLE__SWAP_ALL_BYTES  0x1
#define BV_LCDIF_CTRL_CSC_DATA_SWIZZLE__HWD_SWAP    0x2
#define BV_LCDIF_CTRL_CSC_DATA_SWIZZLE__HWD_BYTE_SWAP   0x3
#define BP_LCDIF_CTRL_LCD_DATABUS_WIDTH      10
#define BM_LCDIF_CTRL_LCD_DATABUS_WIDTH 0x00000C00
#define BF_LCDIF_CTRL_LCD_DATABUS_WIDTH(v)  \
    (((v) << 10) & BM_LCDIF_CTRL_LCD_DATABUS_WIDTH)
#define BV_LCDIF_CTRL_LCD_DATABUS_WIDTH__16_BIT 0x0
#define BV_LCDIF_CTRL_LCD_DATABUS_WIDTH__8_BIT  0x1
#define BV_LCDIF_CTRL_LCD_DATABUS_WIDTH__18_BIT 0x2
#define BV_LCDIF_CTRL_LCD_DATABUS_WIDTH__24_BIT 0x3
#define BP_LCDIF_CTRL_WORD_LENGTH      8
#define BM_LCDIF_CTRL_WORD_LENGTH 0x00000300
#define BF_LCDIF_CTRL_WORD_LENGTH(v)  \
    (((v) << 8) & BM_LCDIF_CTRL_WORD_LENGTH)
#define BV_LCDIF_CTRL_WORD_LENGTH__16_BIT 0x0
#define BV_LCDIF_CTRL_WORD_LENGTH__8_BIT  0x1
#define BV_LCDIF_CTRL_WORD_LENGTH__18_BIT 0x2
#define BV_LCDIF_CTRL_WORD_LENGTH__24_BIT 0x3
#define BM_LCDIF_CTRL_RGB_TO_YCBCR422_CSC 0x00000080
#define BM_LCDIF_CTRL_ENABLE_PXP_HANDSHAKE 0x00000040
#define BM_LCDIF_CTRL_LCDIF_MASTER 0x00000020
#define BM_LCDIF_CTRL_DMA_BURST_LENGTH 0x00000010
#define BM_LCDIF_CTRL_DATA_FORMAT_16_BIT 0x00000008
#define BM_LCDIF_CTRL_DATA_FORMAT_18_BIT 0x00000004
#define BV_LCDIF_CTRL_DATA_FORMAT_18_BIT__LOWER_18_BITS_VALID 0x0
#define BV_LCDIF_CTRL_DATA_FORMAT_18_BIT__UPPER_18_BITS_VALID 0x1
#define BM_LCDIF_CTRL_DATA_FORMAT_24_BIT 0x00000002
#define BV_LCDIF_CTRL_DATA_FORMAT_24_BIT__ALL_24_BITS_VALID   0x0
#define BV_LCDIF_CTRL_DATA_FORMAT_24_BIT__DROP_UPPER_2_BITS_PER_BYTE 0x1
#define BM_LCDIF_CTRL_RUN 0x00000001
#define HW_LCDIF_CTRL1_ADDR (REGS_LCDIF_BASE + 0x00000010)
#define HW_LCDIF_CTRL1_SET(x) ((*(u32 *)(HW_LCDIF_CTRL1_ADDR+4))=x)
#define HW_LCDIF_CTRL1_CLR(x) ((*(u32 *)(HW_LCDIF_CTRL1_ADDR+8))=x)
#define BM_LCDIF_CTRL1_BM_ERROR_IRQ_EN 0x04000000
#define BM_LCDIF_CTRL1_BM_ERROR_IRQ 0x02000000
#define BV_LCDIF_CTRL1_BM_ERROR_IRQ__NO_REQUEST 0x0
#define BV_LCDIF_CTRL1_BM_ERROR_IRQ__REQUEST    0x1
#define BM_LCDIF_CTRL1_RECOVER_ON_UNDERFLOW 0x01000000
#define BM_LCDIF_CTRL1_INTERLACE_FIELDS 0x00800000
#define BM_LCDIF_CTRL1_START_INTERLACE_FROM_SECOND_FIELD 0x00400000
#define BM_LCDIF_CTRL1_FIFO_CLEAR 0x00200000
#define BM_LCDIF_CTRL1_IRQ_ON_ALTERNATE_FIELDS 0x00100000
#define BP_LCDIF_CTRL1_BYTE_PACKING_FORMAT      16
#define BM_LCDIF_CTRL1_BYTE_PACKING_FORMAT 0x000F0000
#define BF_LCDIF_CTRL1_BYTE_PACKING_FORMAT(v)  \
    (((v) << 16) & BM_LCDIF_CTRL1_BYTE_PACKING_FORMAT)
#define BM_LCDIF_CTRL1_OVERFLOW_IRQ_EN 0x00008000
#define BM_LCDIF_CTRL1_UNDERFLOW_IRQ_EN 0x00004000
#define BM_LCDIF_CTRL1_CUR_FRAME_DONE_IRQ_EN 0x00002000
#define BM_LCDIF_CTRL1_VSYNC_EDGE_IRQ_EN 0x00001000
#define BM_LCDIF_CTRL1_OVERFLOW_IRQ 0x00000800
#define BV_LCDIF_CTRL1_OVERFLOW_IRQ__NO_REQUEST 0x0
#define BV_LCDIF_CTRL1_OVERFLOW_IRQ__REQUEST    0x1
#define BM_LCDIF_CTRL1_UNDERFLOW_IRQ 0x00000400
#define BV_LCDIF_CTRL1_UNDERFLOW_IRQ__NO_REQUEST 0x0
#define BV_LCDIF_CTRL1_UNDERFLOW_IRQ__REQUEST    0x1
#define BM_LCDIF_CTRL1_CUR_FRAME_DONE_IRQ 0x00000200
#define BV_LCDIF_CTRL1_CUR_FRAME_DONE_IRQ__NO_REQUEST 0x0
#define BV_LCDIF_CTRL1_CUR_FRAME_DONE_IRQ__REQUEST    0x1
#define BM_LCDIF_CTRL1_VSYNC_EDGE_IRQ 0x00000100
#define BV_LCDIF_CTRL1_VSYNC_EDGE_IRQ__NO_REQUEST 0x0
#define BV_LCDIF_CTRL1_VSYNC_EDGE_IRQ__REQUEST    0x1
#define BM_LCDIF_CTRL1_PAUSE_TRANSFER 0x00000040
#define BM_LCDIF_CTRL1_PAUSE_TRANSFER_IRQ_EN 0x00000020
#define BM_LCDIF_CTRL1_PAUSE_TRANSFER_IRQ 0x00000010
#define BV_LCDIF_CTRL1_PAUSE_TRANSFER_IRQ__NO_REQUEST 0x0
#define BV_LCDIF_CTRL1_PAUSE_TRANSFER_IRQ__REQUEST    0x1
#define BM_LCDIF_CTRL1_LCD_CS_CTRL 0x00000008
#define BM_LCDIF_CTRL1_BUSY_ENABLE 0x00000004
#define BV_LCDIF_CTRL1_BUSY_ENABLE__BUSY_DISABLED 0x0
#define BV_LCDIF_CTRL1_BUSY_ENABLE__BUSY_ENABLED  0x1
#define BM_LCDIF_CTRL1_MODE86 0x00000002
#define BV_LCDIF_CTRL1_MODE86__8080_MODE 0x0
#define BV_LCDIF_CTRL1_MODE86__6800_MODE 0x1
#define BM_LCDIF_CTRL1_RESET 0x00000001
#define BV_LCDIF_CTRL1_RESET__LCDRESET_LOW  0x0
#define BV_LCDIF_CTRL1_RESET__LCDRESET_HIGH 0x1
#define HW_LCDIF_TRANSFER_COUNT_ADDR (REGS_LCDIF_BASE + 0x00000020)
#define BP_LCDIF_TRANSFER_COUNT_V_COUNT      16
#define BM_LCDIF_TRANSFER_COUNT_V_COUNT 0xFFFF0000
#define BF_LCDIF_TRANSFER_COUNT_V_COUNT(v) \
    (((v) << 16) & BM_LCDIF_TRANSFER_COUNT_V_COUNT)
#define BP_LCDIF_TRANSFER_COUNT_H_COUNT      0
#define BM_LCDIF_TRANSFER_COUNT_H_COUNT 0x0000FFFF
#define BF_LCDIF_TRANSFER_COUNT_H_COUNT(v)  \
    (((v) << 0) & BM_LCDIF_TRANSFER_COUNT_H_COUNT)
#define HW_LCDIF_CUR_BUF_ADDR (REGS_LCDIF_BASE + 0x00000030)
#define BP_LCDIF_CUR_BUF_ADDR      0
#define BM_LCDIF_CUR_BUF_ADDR 0xFFFFFFFF
#define BF_LCDIF_CUR_BUF_ADDR(v)   (v)
#define HW_LCDIF_NEXT_BUF_ADDR (REGS_LCDIF_BASE + 0x00000040)
#define BP_LCDIF_NEXT_BUF_ADDR      0
#define BM_LCDIF_NEXT_BUF_ADDR 0xFFFFFFFF
#define BF_LCDIF_NEXT_BUF_ADDR(v)   (v)

#define HW_LCDIF_VDCTRL0_ADDR (REGS_LCDIF_BASE + 0x00000070)
#define HW_LCDIF_VDCTRL0_SET(x) ((*(u32 *)(HW_LCDIF_VDCTRL0_ADDR+4))=x)
#define HW_LCDIF_VDCTRL0_CLR(x) ((*(u32 *)(HW_LCDIF_VDCTRL0_ADDR+8))=x)
#define BM_LCDIF_VDCTRL0_VSYNC_OEB 0x20000000
#define BV_LCDIF_VDCTRL0_VSYNC_OEB__VSYNC_OUTPUT 0x0
#define BV_LCDIF_VDCTRL0_VSYNC_OEB__VSYNC_INPUT  0x1
#define BM_LCDIF_VDCTRL0_ENABLE_PRESENT 0x10000000
#define BM_LCDIF_VDCTRL0_VSYNC_POL 0x08000000
#define BM_LCDIF_VDCTRL0_HSYNC_POL 0x04000000
#define BM_LCDIF_VDCTRL0_DOTCLK_POL 0x02000000
#define BM_LCDIF_VDCTRL0_ENABLE_POL 0x01000000
#define BM_LCDIF_VDCTRL0_VSYNC_PERIOD_UNIT 0x00200000
#define BM_LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH_UNIT 0x00100000
#define BM_LCDIF_VDCTRL0_HALF_LINE 0x00080000
#define BM_LCDIF_VDCTRL0_HALF_LINE_MODE 0x00040000
#define BP_LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH      0
#define BM_LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH 0x0003FFFF
#define BF_LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH(v)  \
        (((v) << 0) & BM_LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH)

#define HW_LCDIF_VDCTRL1_ADDR (REGS_LCDIF_BASE + 0x00000080)
#define HW_LCDIF_VDCTRL1_SET(x) ((*(u32 *)(HW_LCDIF_VDCTRL1_ADDR+4))=x)
#define HW_LCDIF_VDCTRL1_CLR(x) ((*(u32 *)(HW_LCDIF_VDCTRL1_ADDR+8))=x)
#define BP_LCDIF_VDCTRL1_VSYNC_PERIOD      0
#define BM_LCDIF_VDCTRL1_VSYNC_PERIOD 0xFFFFFFFF
#define BF_LCDIF_VDCTRL1_VSYNC_PERIOD(v)   (v)

#define HW_LCDIF_VDCTRL2_ADDR (REGS_LCDIF_BASE + 0x00000090)
#define HW_LCDIF_VDCTRL2_SET(x) ((*(u32 *)(HW_LCDIF_VDCTRL2_ADDR+4))=x)
#define HW_LCDIF_VDCTRL2_CLR(x) ((*(u32 *)(HW_LCDIF_VDCTRL2_ADDR+8))=x)
#define BP_LCDIF_VDCTRL2_HSYNC_PULSE_WIDTH      24
#define BM_LCDIF_VDCTRL2_HSYNC_PULSE_WIDTH 0xFF000000
#define BF_LCDIF_VDCTRL2_HSYNC_PULSE_WIDTH(v) \
        (((v) << 24) & BM_LCDIF_VDCTRL2_HSYNC_PULSE_WIDTH)
#define BP_LCDIF_VDCTRL2_HSYNC_PERIOD      0
#define BM_LCDIF_VDCTRL2_HSYNC_PERIOD 0x0003FFFF
#define BF_LCDIF_VDCTRL2_HSYNC_PERIOD(v)  \
        (((v) << 0) & BM_LCDIF_VDCTRL2_HSYNC_PERIOD)

#define HW_LCDIF_VDCTRL3_ADDR (REGS_LCDIF_BASE + 0x000000a0)
#define HW_LCDIF_VDCTRL3_SET(x) ((*(u32 *)(HW_LCDIF_VDCTRL3_ADDR+4))=x)
#define HW_LCDIF_VDCTRL3_CLR(x) ((*(u32 *)(HW_LCDIF_VDCTRL3_ADDR+8))=x)
#define BM_LCDIF_VDCTRL3_MUX_SYNC_SIGNALS 0x20000000
#define BM_LCDIF_VDCTRL3_VSYNC_ONLY 0x10000000
#define BP_LCDIF_VDCTRL3_HORIZONTAL_WAIT_CNT      16
#define BM_LCDIF_VDCTRL3_HORIZONTAL_WAIT_CNT 0x0FFF0000
#define BF_LCDIF_VDCTRL3_HORIZONTAL_WAIT_CNT(v)  \
        (((v) << 16) & BM_LCDIF_VDCTRL3_HORIZONTAL_WAIT_CNT)
#define BP_LCDIF_VDCTRL3_VERTICAL_WAIT_CNT      0
#define BM_LCDIF_VDCTRL3_VERTICAL_WAIT_CNT 0x0000FFFF
#define BF_LCDIF_VDCTRL3_VERTICAL_WAIT_CNT(v)  \
        (((v) << 0) & BM_LCDIF_VDCTRL3_VERTICAL_WAIT_CNT)

#define HW_LCDIF_VDCTRL4_ADDR (REGS_LCDIF_BASE + 0x000000b0)
#define HW_LCDIF_VDCTRL4_SET(x) ((*(u32 *)(HW_LCDIF_VDCTRL4_ADDR+4))=x)
#define HW_LCDIF_VDCTRL4_CLR(x) ((*(u32 *)(HW_LCDIF_VDCTRL4_ADDR+8))=x)
#define BM_LCDIF_VDCTRL4_SYNC_SIGNALS_ON 0x00040000
#define BP_LCDIF_VDCTRL4_DOTCLK_H_VALID_DATA_CNT      0
#define BM_LCDIF_VDCTRL4_DOTCLK_H_VALID_DATA_CNT 0x0003FFFF
#define BF_LCDIF_VDCTRL4_DOTCLK_H_VALID_DATA_CNT(v)  \
        (((v) << 0) & BM_LCDIF_VDCTRL4_DOTCLK_H_VALID_DATA_CNT)




#define HW_LCDIF_TIMING_ADDR (REGS_LCDIF_BASE + 0x00000060)





/* minimal implementation of string functions */
/*
static char *strstr(const char *s1, const char *s2)
{
	int i;

	if (*s1 == '\0')
		return *s2 ? 0 : (char *)s1;

	while (*s1) {
		for (i = 0; ; i++) {
			if (s2[i] == '\0')
				return (char *)s1;
			if (s2[i] != s1[i])
				break;
		}
		s1++;
	}
	return 0;
}

static int strlen(const char *s)
{
	const char *start = s;

	while (*s)
		s++;

	return s - start;
}

static char *strcpy(char *s1, const char *s2)
{
	char *s = s1;

	while ((*s1++ = *s2++) != '\0')
		;

	return s;
}
*/

static void udelay(int usecs) {
    char c;
    int i;
    usecs*=100;
    for(i=1; i<=usecs; i++) {
        c++;
        c=c/i;
    }
    return;
}

static void *memcpy(void *s1, const void *s2, int n)
{
	char *dst = s1;
	const char *src = s2;

	while (n-- > 0)
		*dst++ = *src++;

	return s1;
}

#define REGS_RTC_BASE		(REGS_BASE + 0x0005C000)

#define HW_RTC_CTRL_ADDR	(REGS_RTC_BASE + 0x00000000)
#define HW_RTC_CTRL_SET_ADDR	(REGS_RTC_BASE + 0x00000004)
#define HW_RTC_CTRL_CLR_ADDR	(REGS_RTC_BASE + 0x00000008)
#define HW_RTC_CTRL_TOG_ADDR	(REGS_RTC_BASE + 0x0000000C)

#define HW_RTC_CTRL		(*(volatile unsigned u32 *) HW_RTC_CTRL_ADDR)
#define HW_RTC_CTRL_RD()	(HW_RTC_CTRL)
#define HW_RTC_CTRL_WR(v)	(HW_RTC_CTRL = (v))
#define HW_RTC_CTRL_SET(v)	((*(volatile unsigned u32 *) HW_RTC_CTRL_SET_ADDR) = (v))
#define HW_RTC_CTRL_CLR(v)	((*(volatile unsigned u32 *) HW_RTC_CTRL_CLR_ADDR) = (v))
#define HW_RTC_CTRL_TOG(v)	((*(volatile unsigned u32 *) HW_RTC_CTRL_TOG_ADDR) = (v))

#define BM_RTC_CTRL_SFTRST	0x80000000
#define BM_RTC_CTRL_CLKGATE	0x40000000

#define HW_RTC_STAT_ADDR	(REGS_RTC_BASE + 0x00000010)
#define HW_RTC_STAT		(*(volatile unsigned u32 *) HW_RTC_STAT_ADDR)
#define HW_RTC_STAT_RD()	(HW_RTC_STAT)

#define BM_RTC_STAT_STALE_REGS      0x00FF0000
#define BF_RTC_STAT_STALE_REGS(v)   (((v) << 16) & BM_RTC_STAT_STALE_REGS)

#define HW_RTC_PERSISTENT1_ADDR      (REGS_RTC_BASE + 0x00000070)
#define HW_RTC_PERSISTENT1           (*(volatile unsigned u32 *) HW_RTC_PERSISTENT1_ADDR)
#define HW_RTC_PERSISTENT1_RD()      (HW_RTC_PERSISTENT1)

#define NAND_SECONDARY_BOOT          0x00000002

/***********************************************************************/


static void lcdif_init(void) {
    *(u32 *)0x80030000=0x000b0800;
    *(u32 *)0x80030010=0x010f0701;
    *(u32 *)0x80030020=0x00f00140;
    *(u32 *)0x80030030=0x43e00000;
    *(u32 *)0x80030040=0x43e00000;
    *(u32 *)0x80030050=0x00000000;
    *(u32 *)0x80030060=0x01010101;
    *(u32 *)0x80030070=0x1c300003;
    *(u32 *)0x80030080=0x000000f9;
    *(u32 *)0x80030090=0x28000179;
    *(u32 *)0x800300a0=0x00340006;
    *(u32 *)0x800300b0=0x00040140;
    *(u32 *)0x800300c0=0x00000000;
    *(u32 *)0x800300d0=0x00000000;
    *(u32 *)0x800300e0=0x00000000;
    *(u32 *)0x800300f0=0x00000000;
    *(u32 *)0x80030100=0x00000000;
    *(u32 *)0x80030110=0x00000000;
    *(u32 *)0x80030120=0x00000000;
    *(u32 *)0x80030130=0x00000000;
    *(u32 *)0x80030140=0x00000000;
    *(u32 *)0x80030150=0x00000000;
    *(u32 *)0x80030160=0x00800010;
    *(u32 *)0x80030170=0x00ff00ff;
    *(u32 *)0x80030180=0x00000030;
    *(u32 *)0x80030190=0x00000000;
    *(u32 *)0x800301a0=0x00000000;
    *(u32 *)0x800301b0=0x00000000;
    *(u32 *)0x800301c0=0x00000000;
    *(u32 *)0x800301d0=0x89000000;
    *(u32 *)0x800301e0=0x03000000;
    *(u32 *)0x800301f0=0x2f030400;
    *(u32 *)0x80030200=0x004000d6;
    return;
}

// Taken from stmp389x_lcdif.c
static void stmp3xxx_init_lcdif(void) {

    // Reset controller
    HW_LCDIF_CTRL_SET(BM_LCDIF_CTRL_SFTRST);
    udelay(10);

    // Take controller out of reset
    HW_LCDIF_CTRL_CLR(BM_LCDIF_CTRL_SFTRST | BM_LCDIF_CTRL_CLKGATE);


    // Setup the bus protocol
    HW_LCDIF_CTRL1_CLR(BM_LCDIF_CTRL1_MODE86);
    HW_LCDIF_CTRL1_CLR(BM_LCDIF_CTRL1_BUSY_ENABLE);

    // Take display out of reset
    HW_LCDIF_CTRL1_SET(BM_LCDIF_CTRL1_RESET);

    // VSYNC is an input by default
    HW_LCDIF_VDCTRL0_SET(BM_LCDIF_VDCTRL0_VSYNC_OEB);

    // Reset display
    HW_LCDIF_CTRL1_CLR(BM_LCDIF_CTRL1_RESET);
    udelay(10);
    HW_LCDIF_CTRL1_SET(BM_LCDIF_CTRL1_RESET);
    udelay(10);

    // Switch display to DOTCLK.
    HW_LCDIF_CTRL_SET(BM_LCDIF_CTRL_DOTCLK_MODE);
    HW_LCDIF_CTRL_SET(BM_LCDIF_CTRL_BYPASS_COUNT);

    // Disable vsync (?)
    HW_LCDIF_VDCTRL0_CLR(BM_LCDIF_VDCTRL0_VSYNC_OEB);
    HW_LCDIF_VDCTRL0_CLR(BM_LCDIF_VDCTRL0_VSYNC_POL);
    HW_LCDIF_VDCTRL0_CLR(BM_LCDIF_VDCTRL0_HSYNC_POL);
    HW_LCDIF_VDCTRL0_CLR(BM_LCDIF_VDCTRL0_DOTCLK_POL);
    HW_LCDIF_VDCTRL0_CLR(BM_LCDIF_VDCTRL0_ENABLE_POL);
    HW_LCDIF_VDCTRL0_SET(BM_LCDIF_VDCTRL0_ENABLE_PRESENT);
    HW_LCDIF_VDCTRL0_SET(BM_LCDIF_VDCTRL0_VSYNC_PERIOD_UNIT);
    HW_LCDIF_VDCTRL0_SET(BM_LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH_UNIT);
    HW_LCDIF_VDCTRL0_CLR(BM_LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH);
    HW_LCDIF_VDCTRL0_SET(BF_LCDIF_VDCTRL0_VSYNC_PULSE_WIDTH(280));

    HW_LCDIF_VDCTRL1_CLR(BM_LCDIF_VDCTRL1_VSYNC_PERIOD);
    HW_LCDIF_VDCTRL1_SET(BF_LCDIF_VDCTRL1_VSYNC_PERIOD(280));

    HW_LCDIF_VDCTRL2_CLR(BM_LCDIF_VDCTRL2_HSYNC_PULSE_WIDTH);
    HW_LCDIF_VDCTRL2_SET(BF_LCDIF_VDCTRL2_HSYNC_PULSE_WIDTH(10));
    HW_LCDIF_VDCTRL2_CLR(BM_LCDIF_VDCTRL2_HSYNC_PERIOD);
    HW_LCDIF_VDCTRL2_SET(BF_LCDIF_VDCTRL2_HSYNC_PERIOD(360));

    HW_LCDIF_VDCTRL3_CLR(BM_LCDIF_VDCTRL3_VSYNC_ONLY);
    HW_LCDIF_VDCTRL3_CLR(BM_LCDIF_VDCTRL3_VERTICAL_WAIT_CNT);
    HW_LCDIF_VDCTRL3_SET(BF_LCDIF_VDCTRL3_VERTICAL_WAIT_CNT(20));
    HW_LCDIF_VDCTRL3_CLR(BM_LCDIF_VDCTRL3_HORIZONTAL_WAIT_CNT);
    HW_LCDIF_VDCTRL3_SET(BF_LCDIF_VDCTRL3_HORIZONTAL_WAIT_CNT(20));

    HW_LCDIF_VDCTRL4_CLR(BM_LCDIF_VDCTRL4_DOTCLK_H_VALID_DATA_CNT);
    HW_LCDIF_VDCTRL4_SET(BF_LCDIF_VDCTRL4_DOTCLK_H_VALID_DATA_CNT(320));
    HW_LCDIF_VDCTRL4_SET(BM_LCDIF_VDCTRL4_SYNC_SIGNALS_ON);



    return;
}

// Taken from stmp389x_lcdif.c
static void stmp3xxx_lcdif_dma_init(void) {
    serial_puts("static void stmp3xxx_lcdif_dma_init(void) {\n");

    serial_puts("    HW_LCDIF_CTRL_SET(BM_LCDIF_CTRL_LCDIF_MASTER);\n");
    HW_LCDIF_CTRL_SET(BM_LCDIF_CTRL_LCDIF_MASTER);

    serial_puts("    *(u32 *)HW_LCDIF_CUR_BUF_ADDR=0x43e00000;\n");
    *(u32 *)HW_LCDIF_CUR_BUF_ADDR=0x43e00000;
    serial_puts("    *(u32 *)HW_LCDIF_NEXT_BUF_ADDR=0x43e00000;\n");
    *(u32 *)HW_LCDIF_NEXT_BUF_ADDR=0x43e00000;
    serial_puts("return;\n");
    return;
}


// Taken from stmp389x_lcdif.c
static void stmp3xxx_lcdif_run(void) {
    serial_puts("static void stmp3xxx_lcdif_run(void) {\n");
    serial_puts("    HW_LCDIF_CTRL_SET(BM_LCDIF_CTRL_LCDIF_MASTER);\n");
    HW_LCDIF_CTRL_SET(BM_LCDIF_CTRL_LCDIF_MASTER);
    serial_puts("    HW_LCDIF_CTRL_SET(BM_LCDIF_CTRL_RUN);\n");
    HW_LCDIF_CTRL_SET(BM_LCDIF_CTRL_RUN);
    serial_puts("return;\n");
    return;
}

static void backlight_init(void) {
#ifdef USB_BACKLIGHT_HACK
#warning This code should be disabled once the backlight is decoupled from the USB system
    udelay(100000);
    ///scripts/regutil -w HW_POWER_5VCTRL=0x00000010
    *(u32 *)0x80044010=0x00000010;

    ///scripts/regutil -w HW_PINCTRL_MUXSEL1_SET=0x00300000
    *(u32 *)0x80018114=0x00000010;

    ///scripts/regutil -w HW_PINCTRL_PULL0_CLR=0x04000000
    *(u32 *)0x80018408=0x40000000;

    ///scripts/regutil -w HW_PINCTRL_DOUT0_CLR=0x04000000
    *(u32 *)0x80018504=0x04000000;

    ///scripts/regutil -w HW_PINCTRL_DOE0_SET=0x04000000
    *(u32 *)0x80018704=0x04000000;

    ///scripts/regutil -w HW_PINCTRL_DOUT0_CLR=0x04000000
    *(u32 *)0x80018504=0x04000000;
#endif

    // Set the backlight PWM to a GPIO.  It's on Bank 1, Pin 28.
    //HW_PINCTRL_MUXSEL3_SET=0x03000000
    *(u32 *)0x80018134=0x03000000;

    // Ensure it's not a pullup.
    //HW_PINCTRL_PULL1_CLR=0x10000000
    *(u32 *)0x80018418=0x10000000;

    // Set the backlight PWM to full.
    //HW_PINCTRL_DOUT1_SET=0x10000000
    *(u32 *)0x80018514=0x10000000;

    // Set this particular pin to be an output.
    //HW_PINCTRL_DOE1_SET=0x10000000
    *(u32 *)0x80018714=0x10000000;

    // Re-send the data-output value.
    //HW_PINCTRL_DOUT1_SET=0x10000000
    *(u32 *)0x80018514=0x10000000;

    return;
}

static void backlight_toggle() {
    *(u32 *)0x8001851C=0x10000000;
}


static void init_timings()
{
    unsigned phase_time;
    unsigned timings;

    *(u32 *)0x80040000=0x00050000;
    *(u32 *)0x80040010=0x800004b1;
    *(u32 *)0x80040020=0x00011001;
    *(u32 *)0x80040030=0x00000003;
    *(u32 *)0x80040040=0x00000001;
    *(u32 *)0x80040050=0x18000001;
    *(u32 *)0x80040060=0x0000001d;
    *(u32 *)0x80040070=0x0000000a;
    *(u32 *)0x80040080=0x80000014;
    *(u32 *)0x80040090=0x80000000;
    *(u32 *)0x800400a0=0x00000102;
    *(u32 *)0x800400b0=0xb8040004;
    *(u32 *)0x800400c0=0xa0000001;
    *(u32 *)0x800400d0=0xc0000000;
    *(u32 *)0x800400e0=0xa0000001;
    *(u32 *)0x800400f0=0x52576116;
    *(u32 *)0x80040100=0x80000000;
    *(u32 *)0x80040110=0x00000100;

    // Just use a phase_time of 1. As optimal as it gets, now.
    phase_time = 1;

    // Program all 4 timings the same
    timings = phase_time;
    timings |= timings << 8;
    timings |= timings << 16;
    *(u32 *)HW_LCDIF_TIMING_ADDR=0x01010101;



}




static void draw_logo_standard(void) {
    int i;
    u16 *memory_address;
    serial_puts("static void draw_logo_standard() {\n");

    serial_puts("    memory_address = (u16 *)(*(u32 *)HW_LCDIF_CUR_BUF_ADDR);\n");
    memory_address = (u16 *)(*(u32 *)HW_LCDIF_CUR_BUF_ADDR);

    serial_puts("    bytes[i] = x;\n");
    for(i=0; i<320*240; i++) {
        *memory_address=i%31;
        memory_address++;
    }
    serial_puts("    return;\n");
    return;
}

int execute_command(char *cmd) {
    if(*cmd == 'x') {
        serial_puts("Exiting\n");
        return 1;
    }


    if(!*cmd) {
        ;
    }
    else if(*cmd == 'h') {
        serial_puts("x - exit\n");
        serial_puts("w offset value - Write a value to an offset\n");
        serial_puts("r offset - Read a value from an offset\n");
    }
    else if(*cmd == 'w') {
        serial_puts("Writing to offset\n");
    }
    else if(*cmd == 'r') {
        serial_puts("Reading from offset\n");
    }
    else {
        serial_puts("Unknown command\n");
    }
    return 0;
}




/* 
 */
u32 draw_logo (void)
{
    //udelay(100000);
//    stmp3xxx_init_lcdif();
//    lcdif_init();
//    stmp3xxx_lcdif_run();
//    backlight_init();
//    enter_shell();
//    draw_logo_standard();

	return 0;
}

