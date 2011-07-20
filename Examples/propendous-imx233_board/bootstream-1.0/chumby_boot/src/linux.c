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
#include "linux.h"
#include "stmp3xxx.h"
#include "platform.h"

static struct tag *params;

/* minimal implementation of string functions */
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

static void *memcpy(void *s1, const void *s2, int n)
{
    char *dst = s1;
    const char *src = s2;

    while (n-- > 0)
        *dst++ = *src++;

    return s1;
}

#define REGS_RTC_BASE       (REGS_BASE + 0x0005C000)

#define HW_RTC_CTRL_ADDR    (REGS_RTC_BASE + 0x00000000)
#define HW_RTC_CTRL_SET_ADDR    (REGS_RTC_BASE + 0x00000004)
#define HW_RTC_CTRL_CLR_ADDR    (REGS_RTC_BASE + 0x00000008)
#define HW_RTC_CTRL_TOG_ADDR    (REGS_RTC_BASE + 0x0000000C)

#define HW_RTC_CTRL     (*(volatile unsigned int *) HW_RTC_CTRL_ADDR)
#define HW_RTC_CTRL_RD()    (HW_RTC_CTRL)
#define HW_RTC_CTRL_WR(v)   (HW_RTC_CTRL = (v))
#define HW_RTC_CTRL_SET(v)  ((*(volatile unsigned int *) HW_RTC_CTRL_SET_ADDR) = (v))
#define HW_RTC_CTRL_CLR(v)  ((*(volatile unsigned int *) HW_RTC_CTRL_CLR_ADDR) = (v))
#define HW_RTC_CTRL_TOG(v)  ((*(volatile unsigned int *) HW_RTC_CTRL_TOG_ADDR) = (v))

#define BM_RTC_CTRL_SFTRST  0x80000000
#define BM_RTC_CTRL_CLKGATE 0x40000000

#define HW_RTC_STAT_ADDR    (REGS_RTC_BASE + 0x00000010)
#define HW_RTC_STAT     (*(volatile unsigned int *) HW_RTC_STAT_ADDR)
#define HW_RTC_STAT_RD()    (HW_RTC_STAT)

#define BM_RTC_STAT_STALE_REGS      0x00FF0000
#define BF_RTC_STAT_STALE_REGS(v)   (((v) << 16) & BM_RTC_STAT_STALE_REGS)

#define HW_RTC_PERSISTENT1_ADDR      (REGS_RTC_BASE + 0x00000070)
#define HW_RTC_PERSISTENT1           (*(volatile unsigned int *) HW_RTC_PERSISTENT1_ADDR)
#define HW_RTC_PERSISTENT1_RD()      (HW_RTC_PERSISTENT1)

#define NAND_SECONDARY_BOOT          0x00000002

/***********************************************************************/


/*
 * Calculate SDRAM size:
 *
 * size = #cs * 4 banks * #columns * #rows * 2 bytes/word
 */

#define MAX_ROW     13
#define MAX_COLUMN  12

u32 get_sdram_size (void) {
    u32 row, column, csmap, size;
    u32 cs = 0;
    
#ifdef SDRAM_SIZE
    return SDRAM_SIZE;
#endif

    row = MAX_ROW - 
          ((HW_DRAM_CTL10_RD() & BM_DRAM_CTL10_ADDR_PINS) >> 16);

    column = MAX_COLUMN -
         ((HW_DRAM_CTL11_RD() & BM_DRAM_CTL11_COLUMN_SIZE) >> 8);

    csmap = HW_DRAM_CTL14_RD() & BM_DRAM_CTL14_CS_MAP;

    /*
     * Calculate the number of 1 bits in csmap
     * x & (x - 1) - clears the least significant 1 bit. 
     */
    while (csmap) {
        csmap &= (csmap - 1);
        cs++;
    }

    size = cs * 4 * (1 << row) * (1 << column) * 2;

    return size;
}

/*
 * ARM atags
 */
void setup_start_tag (void) {
    params = (struct tag*)ATAGS_BASE_ADDRESS;

    params->hdr.tag = ATAG_CORE;
    params->hdr.size = tag_size(tag_core);

    params->u.core.flags = 0;
    params->u.core.pagesize = 0;
    params->u.core.rootdev = 0;

    params = tag_next(params);
}

void setup_mem_tag (void) {
    params->hdr.tag = ATAG_MEM;
    params->hdr.size = tag_size(tag_mem32);

    params->u.mem.start = SDRAM_BASE;
    params->u.mem.size = get_sdram_size();

    params = tag_next(params);
}


void setup_initrd_tag(char *initrd_offset, int initrd_size) {
    params->hdr.tag = ATAG_INITRD2;
    params->hdr.size = tag_size(tag_initrd);

    params->u.initrd.start  = initrd_offset;
    params->u.initrd.size   = initrd_size;

    params = tag_next(params);
}

void setup_cmdline_tag (char *cmdline) {
    u32 len;

    /* Copy unchanged command line to SDRAM */
    strcpy(params->u.cmdline.cmdline, cmdline);

done_copy:
    /* Command line length including '\0' */
    len = strlen(params->u.cmdline.cmdline) + 1;

    params->hdr.tag = ATAG_CMDLINE;

    /* 
     * Tag size should be multiple of 4, so it is needed to round
     * up a command line size.
     */
    params->hdr.size = (sizeof(struct tag_header) + len + 4) >> 2;

    params = tag_next(params);
}

void setup_end_tag (void) {
    params->hdr.tag = ATAG_NONE;
    params->hdr.size = 0;
}

/* 
 * External interface to set up tags in SDRAM
 * Returns a pointer to tag list in SDRAM
 *
 * The following tags are passed to kernel in addition to start and end
 * tags which must always exist:
 *
 * - Memory tag
 * - Command line tag
 */

int run_linux(char *address, char *cmdline, 
        char *initrd_offset, int initrd_size) {
    void (*_linux)(int /*zero*/, int /*machinetype*/, char * /*cmdline*/) = address;

//    enum magic_key magic_key;
//
//    magic_key = get_magic_key();
//    find_command_lines();

    setup_start_tag();
    setup_mem_tag();
    setup_cmdline_tag(cmdline);
    if(initrd_offset)
        setup_initrd_tag(initrd_offset, initrd_size);
    setup_end_tag();

    _linux(0, MACHINE_ID, ATAGS_BASE_ADDRESS);

    /*

    asm("mov r2, %[tags]        \n\t"
        "ldr    r1, =0x6c5      \n\t"   // Store machine id in r1
        "mov    r0, #0          \n\t"   // Zero r0 register
        "mov    lr, %[offset]   \n\t"   // Jump to Linux kernel
        "mov    pc, lr          \n\t"
        ""
        :
        : [tags] "r" (ATAGS_BASE_ADDRESS), [offset] "r" (address)
        );
    */
    return -1;
   // (u32)ATAGS_BASE_ADDRESS;
}

/*
 * Add a raise() function for when divide-by-zeros occur, which should
 * never happen.  This is a pecularity of gnu-eabi, and shouldn't be
 * required on any other platform.
 */
//int raise(int ignored) { return 0; }

