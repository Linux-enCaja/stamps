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


#define PROMPT "\n> "
#define BOOTLOADER_VERSION "v1.0"
#define DEFAULT_WAIT_TIME 3
#define PMEM_SIZE 0x138c


// Hardware-specific register definitions.
#define HW_RTC_MILLISECONDS 0x8005C020 
#define HW_RTC_MILLISECONDS_SET 0x8005C024 
#define HW_RTC_MILLISECONDS_CLR 0x8005C028 
#define HW_RTC_MILLISECONDS_TOG 0x8005C02C
#define HW_RTC_MILLISECONDS_RD() (*(int *)HW_RTC_MILLISECONDS)

#define HW_RTC_PERSISTENT1_RD() (*(int *)0x8005c070)
#define HW_RTC_PERSISTENT2_RD() (*(int *)0x8005c080)
#define HW_RTC_PERSISTENT3_RD() (*(int *)0x8005c090)
#define HW_RTC_PERSISTENT4_RD() (*(int *)0x8005c0a0)
#define HW_RTC_PERSISTENT5_RD() (*(int *)0x8005c0b0)

#define HW_RTC_PERSISTENT1_WR(x) ((*(int *)0x8005c070)=x)
#define HW_RTC_PERSISTENT2_WR(x) ((*(int *)0x8005c080)=x)
#define HW_RTC_PERSISTENT3_WR(x) ((*(int *)0x8005c090)=x)
#define HW_RTC_PERSISTENT4_WR(x) ((*(int *)0x8005c0a0)=x)
#define HW_RTC_PERSISTENT5_WR(x) ((*(int *)0x8005c0b0)=x)

#define HW_RTC_PERSISTENT1_SET(x) ((*(int *)0x8005c074)=x)
#define HW_RTC_PERSISTENT1_CLR(x) ((*(int *)0x8005c078)=x)
#define HW_RTC_PERSISTENT1_TOG(x) ((*(int *)0x8005c07c)=x)

#define HW_PINCTRL_DRIVE9_CLR(x) ((*(int *)0x80018298)=x)
#define HW_PINCTRL_DRIVE10_CLR(x) ((*(int *)0x800182a8)=x)
#define HW_PINCTRL_DRIVE11_CLR(x) ((*(int *)0x800182b8)=x)
#define HW_PINCTRL_DRIVE12_CLR(x) ((*(int *)0x800182c8)=x)
#define HW_PINCTRL_DRIVE13_CLR(x) ((*(int *)0x800182d8)=x)
#define HW_PINCTRL_DRIVE14_CLR(x) ((*(int *)0x800182e8)=x)


#define STR shell_write_str
#define INT shell_write_int
#define HEX shell_write_hex
#define CHR shell_write_chr
#define shell_init serial_init



#include "setup.h"
#include "stmp3xxx.h"
#include "platform.h"
#include "serial.h"
#include "otp.h"
#include "utils.h"
#include "regs-pinctrl.h"

#define EOF 4




void shell_write_chr(char chr) {
    serial_putc(chr);
}

void shell_write_str(char *str) {
    serial_puts(str);
}

void shell_write_hex(int x) {
    serial_puthex(x);
}


void mdelay(int msecs)
{
  while (msecs--)
    udelay(1000);
}

/***********************************************************************/

typedef struct rom_BootInit {
    long *pMem;
    int size;
    int mode;
} rom_BootInit_t;

typedef unsigned char chunk_t[16];
#define PMEM_SIZE 0x138c

int sd_init(rom_BootInit_t *pInit);
chunk_t *sd_next(int *pCount);
int sd_skip(int count);
int sd_stop(void);
int sd_ctrl(int action, void *pArg);







#define NUMBERS "0123456789"
int shell_write_int(int x) {
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






// SHELL_FUNCTIONS

static int sd_initted = 0;
char *sd_pmem;
static rom_BootInit_t init;

static int my_sd_stop() {
    if(!sd_initted)
        return -1;
    sd_initted = 0;
    return sd_stop();
}


static int my_sd_init() {
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
        init.pMem = (long *)sd_pmem;   // Allocated on stack in main().
        init.mode = 9;         // Set to SSP_PORT_1.
        result = sd_init(&init);
        if(!result)
            sd_initted = 1;
        else
            return result;
    }
    return 0;
}





/*
int shell_func_load(char *env, int argc, char **argv) {
    // argc:
    // 0    - "load"
    // 1    - Offset from beginning of partition.  Must be divisibile by 512.
    // 2    - Destination address in memory.
    // 3    - Number of bytes to copy.
    if(argc > 3) {
        int offset, bytes;
        char *destination;
        int result;
        chunk_t *chunks;
        int blocks_to_skip;

        offset      =         simple_strtoul(argv[1], NULL, 0);
        destination = (char *)simple_strtoul(argv[2], NULL, 0);
        bytes       =         simple_strtoul(argv[3], NULL, 0);


        // The bootstream image, and the SD code, begin at offset 0x800,
        // for reasons I've not been able to understand.  Strip that off of
        // the SD card offset right off the bat.
        offset -= 0x00000800;

        STR("Reading "), HEX(bytes),
            STR(" bytes from offset "), HEX(offset),
            STR(" to destination "), HEX((long)destination),
            CHR('\n');


        if(result = my_sd_init()) {
            STR("Unable to load: "), HEX(result), STR("\n");
            return 1;
        }


        if(blocks_to_skip = offset/sizeof(chunk_t)) {
            STR("Skipping "), INT(blocks_to_skip), STR(" chunks\n");
            if(result = sd_skip(blocks_to_skip)) {
                STR("Error while seeking to offset: "),
                    HEX(result), STR("\n");
                return 1;
            }
        }


        // Keep reading as long as we have bytes to read.
        while(bytes > 0) {
            int chunks_to_read, chunk;
            int read_tries = 0;


            // Perform the read.
            while(read_tries < 10000) {
                read_tries++;
                chunks_to_read = bytes/sizeof(chunk_t);
//                STR("Requesting "), INT(chunks_to_read), STR(" chunks...\n");
                chunks = sd_next(&chunks_to_read);
                if(!chunks_to_read) {
                    udelay(10000);
                    continue;
                }
                else
                    break;
            }
            if(read_tries >= 999) {
                STR("Failed to read after "), INT(read_tries), STR(" tries!");
                STR("  Aborting read with "), HEX(bytes), STR(" bytes left.\n");
                return 1;
            }

            if(chunks_to_read < 0) {
                STR("Error while reading bytes: "), HEX(chunks_to_read), STR("\n");
                return 1;
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
    }


    return 0;
}
*/


int shell_func_linux(char *env, int argc, char **argv) {
    char *cmdline = argv[2];
    unsigned char *offset = (unsigned char *)simple_strtoul(argv[1], NULL, 0);


    if(offset) {
        STR("Going to run linux at offset "), HEX((long)offset),
            STR(" with cmdline "), STR(cmdline),
            STR("\n");
        run_linux(offset, cmdline);
        return 1;
    }
    return 0;
}




u32 enter_shell() {
    char sd_pmem_stack[PMEM_SIZE];
    volatile char *ram_addr = ((int *)0x40000000);

    sd_pmem = sd_pmem_stack;

//Lyre mod
    // flash debug LED pin
    // It lives in PWM, bank 1, pin 28. 
    // should be set to GPIO by reset
    HW_PINCTRL_DOUT1_SET(0x10000000);
    HW_PINCTRL_DOE1_SET (0x10000000);
    
    while (1)
    {
      HW_PINCTRL_DOUT1_SET(0x10000000);
      mdelay(500);
      HW_PINCTRL_DOUT1_CLR(0x10000000);
      mdelay(500);
    }
//end Lyre


    // Set up writing (AND READING!) of the serial port.
    serial_init();



    /*
    volatile int *HW_DRAM_CTL13         = ((int *)0x800e0034);
    // Set the latency timings, as appropriate for this board.
    STR("Current latencies: "); HEX(*HW_DRAM_CTL13); STR("\n");
    *HW_DRAM_CTL13 = ((*HW_DRAM_CTL13)&0xffff) | 0x07060000;
    STR("New latencies: "); HEX(*HW_DRAM_CTL13); STR("\n");
    */


    // Redure the drive strength to 12 mA, in an attempt to get around
    // shoddy soldering jobs.
    /*
    HW_PINCTRL_DRIVE9_CLR(0x11111110);
    HW_PINCTRL_DRIVE10_CLR(0x11111111);
    HW_PINCTRL_DRIVE11_CLR(0x11100111);
    HW_PINCTRL_DRIVE12_CLR(0x11111111);
    HW_PINCTRL_DRIVE13_CLR(0x11111111);
    HW_PINCTRL_DRIVE14_CLR(0x00111111);
    */



    // Scan memory four times.  Throw in some more data by adding a
    // monatonically-increasing value to the address, which will exercise
    // the lower two bits.
    int memory_walk  = 0;
    int memory_tries = 0;
    int memory_failed = 0;
//    ram_addr += 32*1024*1024/4; // Begin at 32-megabytes.  We do this
//                                // because the kernel will already be
//                                // loaded at address 0, and we want to test
//                                // unused memory.
    do {
//        for(memory_tries = 0; memory_tries < 4; memory_tries++) {
            int i, megabyte;
            int failures = 0;
            // The memory test will allow this many cells to fail before
            // restarting the test.  This will allow the operator to see errors
            // and manipulate the board to attempt to affect the errors.
            int max_failures = 16;

            memory_walk++;
            STR("Starting memory scan...\n");// ("), INT(memory_tries+1), STR("/4)...\n");

            for(megabyte=8; megabyte<64 && failures < max_failures; megabyte++) {

                for(i=0; i<1024*1024; i+=4) {
                    int *addr = (int *)(ram_addr+i+megabyte*1024*1024);
                    int value = ((int)addr)+memory_walk;
                    (*addr) = value;
                }

                for(i=0; i<1024*1024 && failures < max_failures; i+=4) {
                    int *addr    = (int *)(ram_addr+i+megabyte*1024*1024);
                    int actual   = *addr;
                    int expected = ((int)addr)+memory_walk;
                    if(actual != expected) {
                        STR("FAIL! Address "), HEX(addr), STR(".  Wanted "),
                            HEX(expected), STR(", got "), HEX(actual),
                            STR(" ("), HEX(expected-actual), STR(")\n");
                        failures++;
                        memory_failed = 1;
                    }
                }
            }
//        }
    // Keep looping if the memory test failed.  If it didn't fail, then
    // this will fall through.
    } while(memory_failed);



    // Print out the greeting banner.
    STR("Chumby factory OTP flasher " BOOTLOADER_VERSION "\n");




    // If persistent bit is set, we want to program the OTP fuses.
    if(HW_RTC_PERSISTENT1_RD()&0x08000000) {
        unsigned long key[4];
        int should_exit = 1;
        int wait_time = DEFAULT_WAIT_TIME;
        int start_time;
        key[0] = HW_RTC_PERSISTENT2_RD();
        key[1] = HW_RTC_PERSISTENT3_RD();
        key[2] = HW_RTC_PERSISTENT4_RD();
        key[3] = HW_RTC_PERSISTENT5_RD();

        STR("This encrypted bootloader code simply blows "
                "the MBR and UNENCRYPTED fuses.\n");
        STR("After the fuses are blown, it reboots, and ought to run "
                "from an unencrypted\n");
        STR("image at the beginning of the SD card.\n");
        STR("Process will begin in "),INT(wait_time),STR(" seconds...\n");
        // When wait_time reaches 0, we've timed out.  Give the user that much
        // time to enter the shell, otherwise we'll just continue.
        start_time = HW_RTC_MILLISECONDS_RD()-1000;
        wait_time++;
        while(wait_time > 0) {

            // Decrement the wait counter.
            if(HW_RTC_MILLISECONDS_RD()-start_time > 1000) {
                wait_time--;
                start_time = HW_RTC_MILLISECONDS_RD();
                STR("\rPress any key to abort... "), INT(wait_time);
            }

            // If there are characters in the buffer, pull them off and enter
            // the shell.
            if(serial_tstc()) {
                should_exit = 0;
                serial_getc();
                break;
            }
        }
        STR("\n");
        if(!should_exit) {
            STR("Not burning.  Power off device.\n");
            while(1);
        }


        // Verify there is actually a key.
        if(!key[0] && !key[1] && !key[2] && !key[3]) {
            STR("Indicated we want to burn OTP, but the key was NULL!\n");
            STR("FAIL\n");
            while(1);
        }

        STR("Programming...\n");

        // Blow the "Boot from MBR" and "Boot from unencrypted image" bits.
        otp_write(0x18, (1<<3)|(1<<4));

        otp_write(0x07, key[3]);
        otp_write(0x06, key[2]);
        otp_write(0x05, key[1]);
        otp_write(0x04, key[0]);


        // Blow the fuses that lock the cryptokey.
        otp_write(0x10, (1<<22)|(1<<21)|(1<<5)|(1<<4));


        // Clear out the key.
        HW_RTC_PERSISTENT2_WR(0x00000000);
        HW_RTC_PERSISTENT3_WR(0x00000000);
        HW_RTC_PERSISTENT4_WR(0x00000000);
        HW_RTC_PERSISTENT5_WR(0x00000000);


        // Get rid of the "Burn OTP keys on boot" bit.
        HW_RTC_PERSISTENT1_CLR(0x08000000);


        // Reboot (off of the MBR image now.)
        sys_reboot();
    }
    else {
        STR("AES key not found.  Booting to Linux to initialize it.\n");


        /*
        char *load_commands[] = {
            "load", "0x00100000", "0x40008000", "0x00400000",
        };
        // The Linux kernel was loaded by bootstream.  So just call it.
        if(shell_func_load( NULL, 4, load_commands)) {
            STR("Couldn't load kernel image.\n");
            while(1);
        }
        */


        char *linux_commands[] = {
            "linux", "0x40008000", "\"console=ttyAM0,115200 init=/linuxrc "
                                   "root=/dev/mmcblk0p2 rootfstype=ext3 ro "
                                   "rootwait lcd_panel=lms350 ssp1=mmc "
                                   "sysrq_always_enabled\"",
        };
        if(shell_func_linux(NULL, 3, linux_commands)) {
            STR("Couldn't run Linux.\n");
            while(1);
        }
    }


    return 0;
}

