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
#include <keys.h>
#include <stmp3xxx.h>
#include <arch/platform.h>

static struct tag *params;
static char *cmdlines[MAGIC_KEY_NR];

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
 * Magic key detection
 */
static enum magic_key get_magic_key (void)
{
    u32 keys;
    int magic;
    
    /* Detect pressed keys */
    keys = get_pressed_keys();

    /* 
     * Check if detected keys combination corresponds to 
     * any magic key code
     */
    for (magic = MAGIC_KEY1; magic < MAGIC_KEY_NR; magic++)
        if (magic_keys[magic] == keys)
            return magic;

    return NO_MAGIC_KEY;
}

/*
 * Command lines handling
 *
 * Go over a .cmdlines section. It must contatin \0 terminated
 * command lines storead one by one:
 *
 *  command_line1\0command_line2\0command_line3\0
 *  |                        |
 * cmdlines_start               cmdlines_end
 */
static void find_command_lines (void)
{
    char *start = &cmdlines_start[0];
    char *end = &cmdlines_end[0];
    char *head, *cur; 
    int l;

    head = cur = start;

    for (l = 0; l < MAGIC_KEY_NR && cur < end; l++) {

        /* Find a command line end */
        while (cur < end && *cur++ != '\0') ;

        /* 
         * If there is a garbage or unterminated command
         * line at the end of the section then skip it.
         */
        if (cur == end && *(cur-1) != '\0')
            break;

        cmdlines[l] = head;
        head = cur;
    }

    /*
     * Bind default command line to magic keys not presented
     * in .commandlines
     */
    while (l < MAGIC_KEY_NR)
        cmdlines[l++] = cmdline_def;
}

/*
 * Calculate SDRAM size:
 *
 * size = #cs * 4 banks * #columns * #rows * 2 bytes/word
 */

#define MAX_ROW     13
#define MAX_COLUMN  12

static u32 get_sdram_size (void)
{
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
static void setup_start_tag (void)
{
    params = (struct tag*)ATAGS_BASE_ADDRESS;

    params->hdr.tag = ATAG_CORE;
    params->hdr.size = tag_size(tag_core);

    params->u.core.flags = 0;
    params->u.core.pagesize = 0;
    params->u.core.rootdev = 0;

    params = tag_next(params);
}

static void setup_mem_tag (void)
{
    params->hdr.tag = ATAG_MEM;
    params->hdr.size = tag_size(tag_mem32);

    params->u.mem.start = SDRAM_BASE;
    params->u.mem.size = get_sdram_size();

    params = tag_next(params);
}

static void setup_cmdline_tag (char *cmdline)
{

    u32 len;
    const char *mtdprefix[] = { "root=/dev/mtdblock", "root=ubi0:rootfs"};
    char *o, *m, *s, *rest;
    int nr, i, j, p, digits;
    unsigned int val;

    /* is a nand root present in the command line? */
    for (p = 0; p < sizeof(mtdprefix)/sizeof(mtdprefix[0]); p++) {
        m = strstr(cmdline, mtdprefix[p]);
        if (m)
            break;
    }

    if (!m)
        goto normal_copy;
    
    /* yes, we have to read the persistent bit now */

    /* start the RTC block */
    HW_RTC_CTRL_CLR(BM_RTC_CTRL_SFTRST | BM_RTC_CTRL_CLKGATE);

    while (HW_RTC_STAT_RD() & BF_RTC_STAT_STALE_REGS(0x1 << 1))
        ;
    val = HW_RTC_PERSISTENT1_RD();

    /* normal boot, no need to modify cmdline */
    if ((val & NAND_SECONDARY_BOOT) == 0)
        goto normal_copy;

    /* add one to the rootfs partition number
     * we assume that the secondary rootfs partition
     * is the next one
     */
    o = params->u.cmdline.cmdline;
    m += strlen(mtdprefix[p]);
    memcpy(o, cmdline, m - cmdline);
    o += m - cmdline;

    s = m;
    while (*s && *s >= '0' && *s <= '9')
        s++;

    rest = s;

    s--;
    nr = 0;
    while (s >= m) {
        nr = nr * 10 + (*s - '0');
        s--;
    }

    nr++;
    digits = 0;
    i = nr;
    while (i > 0) {
        digits++;
        i /= 10;
    }

    i = nr;
    j = digits;
    while (j > 0) {
        o[j - 1] = '0' + (i % 10);
        i /= 10;
        j--;
    }
    o += digits;
    strcpy(o, rest);

    goto done_copy;

normal_copy:
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

static void setup_end_tag (void)
{
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
u32 setup_tags (void)
{
    enum magic_key magic_key;

    magic_key = get_magic_key();
    find_command_lines();

    setup_start_tag();
    setup_mem_tag();
    setup_cmdline_tag(cmdlines[magic_key]);
    setup_end_tag();

    return (u32)ATAGS_BASE_ADDRESS;
}

/*
 * Add a raise() function for when divide-by-zeros occur, which should
 * never happen.  This is a pecularity of gnu-eabi, and shouldn't be
 * required on any other platform.
 */
int raise(int ignored) { return 0; }
