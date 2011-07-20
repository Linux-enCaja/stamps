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

#include "setup.h"
#include "stmp3xxx.h"
#include "platform.h"
#include "serial.h"
#include "utils.h"
#include "sd.h"
#include "regs-power.h"
#define EOF 4


#define PROMPT "\npreboot> "
#define BOOTLOADER_VERSION "v1.0"
#define DEFAULT_WAIT_TIME 2

// Hardware-specific register definitions.
#define HW_RTC_MILLISECONDS 0x8005C020 
#define HW_RTC_MILLISECONDS_SET 0x8005C024 
#define HW_RTC_MILLISECONDS_CLR 0x8005C028 
#define HW_RTC_MILLISECONDS_TOG 0x8005C02C
#define HW_RTC_MILLISECONDS_RD() (*(int *)HW_RTC_MILLISECONDS)

#define STR shell_write_str
#define INT shell_write_int
#define HEX shell_write_hex
#define CHR shell_write_chr
#define shell_init serial_init
#define HEXDUMP shell_write_hexdump


// Load addresses can be handy for finding places to load code.
#define LINUX_LOAD_ADDRESS  0x40008000
#define LINUX_LOAD_SIZE     0x00400000
#define LINUX_k1_LOAD_SOURCE 0x00100000
#define LINUX_k2_LOAD_SOURCE 0x00500000
#define LINUX_k3_LOAD_SOURCE 0x00900000
#define INITRD_LOAD_ADDRESS 0x40408000 //(LINUX_LOAD_ADDRESS+LINUX_LOAD_SIZE)
#define INITRD_LOAD_SIZE    0x00400000
#define INITRD_LOAD_SOURCE  0x00b00000
#define CHUMBY_LOAD_ADDRESS 0x40808000 //(INITRD_LOAD_ADDRESS+INITRD_LOAD_SIZE)
#define CHUMBY_LOAD_SIZE    0x000f7800
#define CHUMBY_LOAD_SOURCE  0x00008800






/***********************************************************************/





static int shell_write_str(char *str) {
     serial_puts(str);
     return 0;
}

static int shell_write_hex(int x) {
    serial_puthex(x);
    return 0;
}

static int shell_write_chr(char c) {
    serial_putc(c);
    return 0;
}

#define NUMBERS "0123456789"
static int shell_write_int(int x) {
    int magnitude = 1;
    // If negative, write the negative sign and make positive.
    if(x<0) {
        shell_write_chr('-');
        x = -x;
    }

    // Figure out the magnitude of the number.
    while(10*magnitude <= x)
        magnitude *= 10;

    // Now print out the number.
    while(magnitude > 0) {
        int digit = x/magnitude;
        shell_write_chr(NUMBERS[digit]);
        x -= digit*magnitude;
        magnitude /= 10;
    }
    return 0;
}


#define HEXDIGITS "0123456789abcdef"
static int shell_write_hexdump(unsigned char *c, int size) {
    int offset = 0;
    int abs_offset = 0;
    while(offset < size) {

        // On the first pass around, print out the absolute address.
        if(!offset)
            HEX(abs_offset), STR("  ");

        // Always print out the hex values.
        CHR(HEXDIGITS[(c[offset]>>4)&0x0f]),
        CHR(HEXDIGITS[(c[offset]>>0)&0x0f]),
        CHR(' ');

        // On byte 7 and 15 (e.g. the last bytes), add an extra space.
        if(offset == 7 || offset == 15)
            CHR(' ');

        // On the very last byte, print out an ascii reprepsentation.
        if(offset == 15) {
            int offset_offset;
            CHR('|');
            for(offset_offset = 0; offset_offset <= offset; offset_offset++) {
                unsigned char ch;
                ch = c[offset_offset];
                if(isprint(ch) && ch < 0x80)
                    CHR(ch);
                else
                    CHR('.');
            }
            STR("|\n");

            size       -= 16;
            offset      = -1;
            abs_offset += 16;
            c          += 16;
        }
        offset++;
    }
    CHR('\n');
    if(offset)
        STR("Had "), INT(offset), STR(" characters left in buffer!\n");
    return 0;
}




static int sd_initted = 0;
char *sd_pmem;

static int my_sd_stop() {
    if(!sd_initted)
        return -1;
    sd_initted = 0;
    return sd_stop();
}


static int my_sd_init() {
    static rom_BootInit_t init;
    if(sd_initted) {
        int result;
        result = my_sd_stop();
        if(result) {
            STR("Error!  Couldn't reset SD.\n");
            return 0;
        }
    }
    if(!sd_initted) {
        int result;
        init.size = PMEM_SIZE;
        init.pMem = (long *)sd_pmem;
        init.mode = 9;  // Set to SSP_PORT_1.
        result = sd_init(&init);
        if(!result)
            sd_initted = 1;
        else
            return result;
    }
    return 0;
}




#include "sd_config_block.h"

#define CFG_OFFSET 0x0000c000

static int real_load(int offset, char *destination, int bytes) {
    int result;
    chunk_t *chunks;
    int blocks_to_skip;

    // The bootstream image, and the SD code, begin at offset 0x800,
    // for reasons I've not been able to understand.  Strip that off of
    // the SD card offset right off the bat.
    offset -= 0x00000800;

    STR("Reading "), HEX(bytes), STR(" bytes from offset "), HEX(offset), STR(" to destination "), HEX((long)destination), CHR('\n');


    result = my_sd_init();
    if(result) {
        STR("Unable to load: "), HEX(result), STR("\n");
        return 0;
    }


    blocks_to_skip = offset/sizeof(chunk_t);
    if(blocks_to_skip) {
        STR("Skipping "), INT(blocks_to_skip), STR(" chunks\n");
        result = sd_skip(blocks_to_skip);
        if(result) {
            STR("Error while seeking to offset: "),
                HEX(result), STR("\n");
            return 0;
        }
    }


    // Keep reading as long as we have bytes to read.
    while(bytes > 0) {
        int chunks_to_read, chunk;
        int read_tries = 0;


        // Perform the read.
        while(read_tries < 1000) {
            read_tries++;
            chunks_to_read = bytes/sizeof(chunk_t);
            chunks = sd_next(&chunks_to_read);
            if(!chunks_to_read) {
                continue;
            }
            else
                break;
        }
        if(read_tries >= 999) {
            STR("Failed to read after "), INT(read_tries), STR(" tries!");
            STR("  Aborting read.\n");
            return 0;
        }

        if(chunks_to_read < 0) {
            STR("Error while reading bytes: "), HEX(chunks_to_read), STR("\n");
            return 0;
        }

        // Copy over each of the chunks that was read.
        for(chunk=0; chunk<chunks_to_read; chunk++) {
            memcpy(destination, chunks[chunk], sizeof(chunk_t));
            destination += sizeof(chunk_t);
            bytes       -= sizeof(chunk_t);
            offset      += sizeof(chunk_t);
        }
    }

    my_sd_stop();

    return 0;
}

struct config_area *sd_cfg;
static int sd_cfg_loaded;
int config_block_load() {
    if(!sd_cfg_loaded) {
        sd_cfg = (char *)0x45A00000;
        if(real_load(CFG_OFFSET, (char *)sd_cfg, sizeof(struct config_area)))
            return 0;
        sd_cfg_loaded = 1;
    }
    return !sd_cfg_loaded;
}

static int config_locate_block(char *block_name, int *offset, int *size) {
    int block;

    *offset = 0;
    *size   = 0;

    // The first time around, we should read out the config block.
    if(config_block_load())
        return 1;

    // Look through the list of blocks trying to find a name that matches.
    // We only consider the first 4 characters or so of the name.
    for(block=0;
        block<sizeof(sd_cfg->block_table)/sizeof(struct block_def);
        block++) {
        if(!strncmp(sd_cfg->block_table[block].n.name,
                    block_name,
                    sizeof(sd_cfg->block_table[block].n.name))) {
            *offset = sd_cfg->block_table[block].offset;
            *size   = sd_cfg->block_table[block].length;

            // Add on the offset of partition 1.  And subtract the 0x800
            // offset that the SD laoder system adds on.
            *offset += sd_cfg->p1_offset - 0x800;
            return 0;
        }
    }
    return 0;
}



int shell_func_load(int argc, char **argv) {
    // argc:
    // 0    - "load"
    // 1    - Offset from beginning of partition.  Must be divisibile by 512.
    // 2    - Destination address in memory.
    // 3    - Number of bytes to copy.
    if(argc > 2) {
        int offset, bytes = 0;
        char *destination;

        offset      =         simple_strtoul(argv[1], NULL, 0);
        destination = (char *)simple_strtoul(argv[2], NULL, 0);
        if(argc > 3)
            bytes   =         simple_strtoul(argv[3], NULL, 0);

        // If the offset is 0, then it might be the name of a block to
        // load.
        if(!offset)
            config_locate_block(argv[1], &offset, &bytes);

//        STR("Args:\n");
//        STR("    offset      : "), HEX(offset), CHR('\n');
//        STR("    destination : "), HEX(destination), CHR('\n');
//        STR("    bytes       : "), HEX(bytes), CHR('\n');

        // If we don't have any bytes to load, just return.
        if(!bytes)
            return 0;


        return real_load(offset, destination, bytes);

    }


    else {
        STR("  "), STR(argv[0]), STR(" "),
            STR("[offset] [destination] [bytes]\n");
        STR("        Copies [bytes] from SD offset [offset] to RAM location [destination]\n");
    }

    return 0;
}



static int shell_func_call(int argc, char **argv) {
    if(argc > 1) {
        int *p     = (int *)simple_strtoul(argv[1], NULL, 0);
        int (*t)() = p;
        STR("Jumping to address "), HEX(p),  STR(":\n");
        HEXDUMP((char *)p, 512);
        STR("Go!\n");
        t();
    }
    else
        STR("Usage: call [address]");
    return 0;
}




// 1) Enables the DCDC.
// 2) Configures the 4.2V regulator (if necessary?)
// 3) Disables all linear regulators.
// 4) Ensures the VDDMEM regulator is enabled.
// 5) Enables the battery charger, if necessary.
// 6) Checks persistent bits to determine if we need to boot or not.  If
//    the persistent bit is set, we'll just charge the battery.
static int setup_power() {
    STR("Setting up power management...\n");
    /*
    volatile int *HW_POWER_CTRL      = ((int *)(0x80044000+0x000));
//    volatile int *HW_POWER_CTRL_SET  = ((int *)0x80044000+0x004);
//    volatile int *HW_POWER_CTRL_CLR  = ((int *)0x80044000+0x008);
    volatile int *HW_POWER_5VCTRL    = ((int *)(0x80044000+0x010)); //
//    volatile int *HW_POWER_5VCTRL_SET= ((int *)0x80044000+0x014);
//    volatile int *HW_POWER_5VCTRL_CLR= ((int *)0x80044000+0x018);
//    volatile int *HW_POWER_5VCTRL_TOG= ((int *)0x80044000+0x01c);
    volatile int *HW_POWER_MINPWR    = ((int *)(0x80044000+0x020)); //
    volatile int *HW_POWER_CHARGE    = ((int *)(0x80044000+0x030)); //
    volatile int *HW_POWER_VDDDCTRL  = ((int *)(0x80044000+0x040)); //
    volatile int *HW_POWER_VDDACTRL  = ((int *)(0x80044000+0x050)); //
    volatile int *HW_POWER_VDDIOCTRL = ((int *)(0x80044000+0x060)); //
    volatile int *HW_POWER_VDDMEMCTRL= ((int *)(0x80044000+0x070)); //
    volatile int *HW_POWER_DCDC4P2   = ((int *)(0x80044000+0x080)); //
    volatile int *HW_POWER_MISC      = ((int *)(0x80044000+0x090)); //
    volatile int *HW_POWER_DCLIMITS  = ((int *)(0x80044000+0x0a0)); //
    volatile int *HW_POWER_LOOPCTRL  = ((int *)(0x80044000+0x0b0)); //
    volatile int *HW_POWER_STS       = ((int *)(0x80044000+0x0c0)); //
    volatile int *HW_LRADC_CTRL0     = ((int *)0x80050000);
    volatile int *HW_LRADC_CONVERSION= ((int *)0x80050130);



    // Turn on the power management block.
    (*HW_POWER_CTRL) = 0x00000000;


    // Turn off all minimum power hacks.
    (*HW_POWER_MINPWR) = 0x00000000;






    // Enable battery voltage monitoring.
    (*HW_LRADC_CTRL0)     =0x00000080;
    (*HW_LRADC_CONVERSION)=0x00120080;
    udelay(50);



    // Configure VDD4P2, even if we're running off battery.  We'll fix this
    // once we're running in Linux.
    // Have it run off of the 5V if present.
    STR("Enable DCDC4P2: ");
    HEX(*HW_POWER_DCDC4P2);
    STR(" -> ");
//    (*HW_POWER_DCDC4P2)=0xd0c00000;
    (*HW_POWER_DCDC4P2)=0x00c00018;
    HEX(*HW_POWER_DCDC4P2);
    STR("\n");

    // Enable the DCDC.
    STR("Enable DCDC: ");
    HEX(*HW_POWER_5VCTRL);
    STR(" -> ");
    (*HW_POWER_5VCTRL)=0x0003f027;
    HEX(*HW_POWER_5VCTRL);
    STR("\n");

    mdelay(10);



    STR("Value of HW_POWER_STS: "), HEX(*HW_POWER_STS), STR("\n");

    // Ensure VDDMEM is enabled, though.
    STR("Enabling VDDMEM\n");
    //0x00000110
    (*HW_POWER_VDDMEMCTRL)=0x00000302;

    mdelay(10);

    // Disable the power charger (for now).
    STR("Disabling the charger\n");
    (*HW_POWER_CHARGE) = 0x00020000;



    mdelay(100);



    // Disable all linear regulators.
    STR("Setting VDDD to use DCDC rather than the linear regulator: ");
    HEX(*HW_POWER_VDDDCTRL);
    STR(" -> ");
    *HW_POWER_VDDDCTRL=0x0072041d;
    HEX(*HW_POWER_VDDDCTRL);
    mdelay(10);
    STR(" -> ");
    *HW_POWER_VDDDCTRL=0x0000041d;
    HEX(*HW_POWER_VDDDCTRL);
    STR("\n");

    mdelay(10);

    STR("Setting VDDA to use DCDC rather than the linear regulator: ");
    HEX(*HW_POWER_VDDACTRL);
    STR(" -> ");
    *HW_POWER_VDDACTRL=0x0002000c;
    HEX(*HW_POWER_VDDACTRL);
    STR("\n");

    mdelay(10);

    STR("Disabling VDDIO: ");
    HEX(*HW_POWER_VDDIOCTRL);
    STR(" -> ");
    *HW_POWER_VDDIOCTRL=0x00020714;
    HEX(*HW_POWER_VDDIOCTRL);
    STR("\n");

    mdelay(10);



    STR("Setting MISC values\n");
    HEX(*HW_POWER_MISC);
//    (*HW_POWER_MISC) = 0x00000000;


    STR("Setting DCDC limits\n");
    HEX(*HW_POWER_DCLIMITS), STR("\n");
//    (*HW_POWER_DCLIMITS) = 0x00000c5f;



    STR("Setting Loop control values\n");
    *HW_POWER_LOOPCTRL=0x00000001;




    // Enable the battery charger.


    // Check the persistent bit, and don't do anything if we're plugged in.


    */
    return 0;
}


u32 enter_shell() {
    char stack_pmem[PMEM_SIZE];
    volatile int *HW_CLKCTRL_EMI  = ((int *)0x80040000+0x0a0);
    volatile int *HW_AUDIOOUT_CTRL      = ((int *)0x80048000);
    volatile int *HW_AUDIOOUT_PWRDN     = ((int *)0x80048070);
    volatile int *HW_AUDIOOUT_ANACTRL   = ((int *)0x80048090);
    /*
    int go_interactive = 0;
    int wait_time = DEFAULT_WAIT_TIME;
    int start_time;
    char line[512];
    int *HW_DRAM_CTL13   = ((int *)0x800e0034);
    int *HW_CLKCTRL_HBUS = ((int *)0x80040000+0x030);
    int *HW_CLKCTRL_CPU  = ((int *)0x80040000+0x020);
    int *HW_CLKCTRL_EMI  = ((int *)0x80040000+0x0a0);
    int *HW_CLKCTRL_FRAC = ((int *)0x80040000+0x0f0);

    int *base_ptr = ((int *)0x80000000);
    int *end_ptr  = ((int *)0x800E2000);
    */

    sd_pmem = stack_pmem;





    // Set up writing (AND READING!) of the serial port.
    shell_init();


    // As a recommendation from Freescale, we turn on SYNC_MODE, which
    // seems to improve performance slightly.
    int emi = *HW_CLKCTRL_EMI;
    emi |= 0x40000000;
    *HW_CLKCTRL_EMI = emi;

    /*
    // Set up DRAM latencies.
    STR("Current latencies: "); HEX(*HW_DRAM_CTL13); STR("\n");
    *HW_DRAM_CTL13 = ((*HW_DRAM_CTL13)&0xffff) | 0x08080000;
    STR("New latencies: "); HEX(*HW_DRAM_CTL13); STR("\n");
    STR("HCLK:    "), HEX(*HW_CLKCTRL_HBUS), STR("\n");
    STR("CPUCLK:  "), HEX(*HW_CLKCTRL_CPU), STR("\n");
    STR("EMICLK:  "), HEX(*HW_CLKCTRL_EMI), STR("\n");
    STR("CLKFRAC: "), HEX(*HW_CLKCTRL_FRAC), STR("\n");
    */


    // Bring up the audio just enough to stop ground loop buzz.
    *HW_AUDIOOUT_CTRL=0x00000061;
    *HW_AUDIOOUT_PWRDN=0x00000000;
    *HW_AUDIOOUT_ANACTRL=0x00000010;


    /*
    STR("\nTesting RAM 1: ");
    int *ptr1 = (int *)0x40808000;
    *ptr1 = 0x12345678;
    if(*ptr1 != 0x12345678)
        STR("PTR not correct!  0x12345678 != "), HEX(*ptr1);
    else
        STR("Okay");

    STR("\nTesting RAM 2: ");
    int *ptr2 = (int *)0x40800000;
    *ptr2 = 0x9abcdef0;
    if(*ptr2 != 0x9abcdef0)
        STR("PTR not correct!  0x9abcdef0 != "), HEX(*ptr2);
    else
        STR("Okay");
    STR("\n");
    */


    //setup_power();


    STR("Loading SD image...\n");
    // Load the bootlaoder from the SD card into RAM.
    char *load_args[] = {"load", "boot", "0x40808000"};
    shell_func_load(4, load_args);


    // Jump to the bootloader.  This should never return.
    char *call_args[] = {"call", "0x40808000"};
    shell_func_call(2, call_args);



    // This shouldn't ever happen.
    STR("Warning: fell off the end of the bootloader!\n");
    return 0;
}


