
#include "linux.h"
#include "stmp3xxx.h"
#include "platform.h"
#include "serial.h"
#include "otp.h"
#include "sd.h"
#include "linux.h"
#include "shell.h"
#include "fb.h"
#include "regs-lcdif.h"
#include "regs-pinctrl.h"
#include "regs-clkctrl.h"
#include "utils.h"
#include "regs-aeskey.h"
#include "sd_config_block.h"
#include "shell_funcs.h"

#define CFG_OFFSET 0x0000c000

struct reg_info {
    char name[40];  // Largest name I've measured is 38 characters.
    int  offset;
};








int shell_func_help(struct shell_env *env, int argc, char **argv) {
//    int cmd;
    STR("Available commands:\n");
    struct shell_command *cmd = env->shell_commands;
    while(cmd->name) {
        STR("    "), STR(cmd->name), STR("\n");
        cmd++;
    }
    return 0;
}



int shell_func_unknown(struct shell_env *env, int argc, char **argv) {
    if(strcmp(argv[0], "")) {
        STR("Unknown command: ");
        STR(argv[0]);
    }
    return 0;
}



int shell_func_exit(struct shell_env *env, int argc, char **argv) {
    env->should_exit = 1;
    return 0;
}



int shell_func_print(struct shell_env *env, int argc, char **argv) {
    int i;
    for(i=0; i<argc; i++) {
        STR("[");
        STR(argv[i]);
        STR("] ");
    }
    return 0;
}



// Bank 3, word 0 contains ENABLE_UNENCRYPTED_BOOT and SD_MBR_BOOT.
// It's located at offset 0x18.
int shell_func_otp(struct shell_env *env, int argc, char **argv) {
    if(argc > 3 && !strcmp(argv[1], "-w")) {
        int address, data;

        address = simple_strtoul(argv[2], NULL, 0);
        data    = simple_strtoul(argv[3], NULL, 0);

        if(address > 0x1F) {
            STR("Unable to write: Address "),HEX(address),STR(" out of range.");
            return 0;
        }


        int result;
        STR("Programing... ");
        result = otp_write(address, data);

        if(1==result)
            STR("CPU isn't running at 480 MHz.  Don't know how to program OTP.\n");
        else if(2==result)
            STR("Couldn't close registers for reading.  Unable to program.\n");
        else if(3==result)
            STR("Error in OTP module.\n");
        else if(0==result)
            STR("done.\n");
        else
            STR("Unknown error occurred.\n");

    }
    
    // Open up all banks for reading.
    else if(argc > 1 && !strcmp(argv[1], "-o")) {
        otp_open_read();
        STR("OTP banks opened for reading.\n");
    }

    else if(argc > 1 && !strcmp(argv[1], "-c")) {
        otp_close_read();
        STR("OTP banks closed for reading.\n");
    }

    else {
        STR(argv[0]), STR(" options:\n");
        STR("  "), STR(argv[0]), STR(" "),
            STR("-w [fuse] [value]  Writes [value] to fuse number [fuse]\n");
        STR("  "), STR(argv[0]), STR(" "),
            STR("-r [fuse]          Reads value from fuse number [fuse]\n");
        STR("  "), STR(argv[0]), STR(" "),
            STR("-l                 ReLoads shadowed register banks\n");
        STR("  "), STR(argv[0]), STR(" "),
            STR("-o                 Opens all banks for reading\n");
        STR("  "), STR(argv[0]), STR(" "),
            STR("-c                 Close unshadowed banks for reading\n");
    }
    return 0;
}


int shell_func_reboot(struct shell_env *env, int argc, char **argv) {
    STR("Rebooting...");
    sys_reboot();

    // Code should never reach here.
    STR("Unable to reboot.\n");
    return 1;
}


static struct reg_info regs[] = {
    #include "regutil_falconwing.h"
    {"\0", 0},
};
int shell_func_regutil(struct shell_env *env, int argc, char **argv) {
    int ret = 0;

    // Dump all registers.
    if(argc > 1 && !strcmp(argv[1], "-d")) {
        struct reg_info *reg = regs;
        while(reg->name && reg->name[0]) {
            if(argc <= 2 || !strncmp(reg->name, argv[2], strlen(argv[2]))) {
                HEX(reg->offset), STR(": "), HEX(*((int *)reg->offset));
                STR("  "), STR(reg->name), STR("\n");
            }
            reg++;
        }
    }

    // Read specific register.
    else if(argc == 3 && !strcmp(argv[1], "-r")) {
        int read_offset = 0;
        if(!read_offset) {
            struct reg_info *reg = regs;
            // Since we changed to arrays-of-chars, reg->name should always
            // be defined.  However, a zero-length string indicates the
            // end-of-list, so continue until that occurs.
            while(reg->name && reg->name[0]) {
                if(!strcmp(reg->name, argv[2])) {
                    read_offset = reg->offset;
                    break;
                }
                reg++;
            }
        }

        if(!read_offset)
            read_offset = simple_strtoul(argv[2], NULL, 0);

        STR("Value at "), HEX(read_offset), STR(": ");
        ret = *((int *)read_offset);
        HEX(ret), STR("\n");
    }

    // Write specific register.
    else if(argc == 4 && !strcmp(argv[1], "-w")) {
        int write_offset = 0;
        int write_value  = 0;

        write_value  = simple_strtoul(argv[3], NULL, 0);

        if(!write_offset) {
            struct reg_info *reg = regs;
            while(reg->name && reg->name[0]) {
                if(!strcmp(reg->name, argv[2])) {
                    write_offset = reg->offset;
                    break;
                }
                reg++;
            }
        }

        if(!write_offset)
            write_offset = simple_strtoul(argv[2], NULL, 0);

        // Actually set the value.
        STR("Setting "), HEX(write_offset), STR(": ");
        HEX(*((int *)write_offset)), STR(" -> ");
        (*((int *)write_offset)) = write_value;
        ret = *((int *)write_offset);
        HEX(ret), STR(" ok\n");
    }

    else {
        STR("Regutil usage:\n");
        STR("  regutil -r [register-or-offset]         Read an area of RAM\n");
        STR("  regutil -w [register-or-offset] [value] Write an area of RAM\n");
        STR("  regutil -d <prefix>                     Dump all registers that start with <prefix>\n");
    }

    return ret;
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
    static rom_BootInit_t init;// = (rom_BootInit_t *)0x40A10000;
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
        init.pMem = sd_pmem;   // Allocated on stack in main().
        init.mode = 9;         // Set to SSP_PORT_1.
        result = sd_init(&init);
        if(!result)
            sd_initted = 1;
        else
            return result;
    }
    return 0;
}


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
//            STR("Going to try to read "), INT(chunks_to_read), STR(" chunks\n");
        while(read_tries < 1000) {
            read_tries++;
            chunks_to_read = bytes/sizeof(chunk_t);
            chunks = sd_next(&chunks_to_read);
            if(!chunks_to_read) {
//                    STR("Read 0 chunks.  Trying again...\n");
//                    mdelay(1000*read_tries);  // XXX need to tune this!
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
//            else if(read_tries)
//                STR("Delayed "), INT(read_tries), STR(" times.\n");

        if(chunks_to_read < 0) {
            STR("Error while reading bytes: "), HEX(chunks_to_read), STR("\n");
            return 0;
        }
//            else
//                STR("Successfully read "), INT(chunks_to_read), STR(" chunks.\n");

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

            // Not sure where this constant came from, but it seems required.
            //*offset -= 0xc3000;
            *offset += sd_cfg->p1_offset - 0x800;
            return 0;
        }
    }
    return 0;
}



int shell_func_sd(struct shell_env *env, int argc, char **argv) {


    // sd init
    if(argc > 1 && !strcmp(argv[1], "init")) {
        int result;
        STR("my_sd_init(): ");
        result = my_sd_init();
        if(!result) {
            STR("ok\n");
        }
        else {
            STR("error "), HEX(result), STR("\n");
        }
    }


    // sd next
    else if(argc > 2 && !strcmp(argv[1], "next")) {
        int count;
        chunk_t *chunks;

        count = simple_strtoul(argv[2], NULL, 0);
        STR("sd_next(&"), INT(count), STR("): ");
        chunks = sd_next(&count);

        if(!chunks && count < 0) {
            STR("error "), HEX(count), STR("\n");
        }
        else {
            int i;
            INT(count), STR(" chunks fetched\n");
            for(i=0; i<count; i++) {
                STR("Chunk "), INT(i), STR(":\n");
                HEXDUMP(chunks[i], sizeof(chunks[i]));
            }
        }
    }


    // sd skip
    else if(argc > 2 && !strcmp(argv[1], "skip")) {
        // Call "skip" function.
        int result;
        int count;

        count = simple_strtoul(argv[2], NULL, 0);
        STR("sd_skip("), INT(count), STR("): ");
        result = sd_skip(count);
        if(result) {
            STR("error "), HEX(result), STR("\n");
        }
        else {
            STR("ok\n");
        }
    }


    // sd stop
    else if(argc > 1 && !strcmp(argv[1], "stop")) {
        // Call "stop" function.
        int result;

        STR("sd_stop(): ");
        result = my_sd_stop();
        if(result) {
            STR("error "), HEX(result), STR("\n");
        }
        else {
            STR("ok\n");
        }
    }
    else if(argc > 1 && !strcmp(argv[1], "ctrl")) {
        int result;
        // Call "ctrl" function.
        STR("sd_ctrl(): ");
        result = sd_ctrl(0, NULL);
        INT(result), STR("\n");
    }
    else {
        STR("sd usage:\n");
        STR("  "), STR(argv[0]), STR(" "),
            STR("init                   Sets up SD system\n");
        STR("  "), STR(argv[0]), STR(" "),
            STR("next [count]           Reads [count] 16-byte blocks "
                    "from the SD card, then skips 16 bytes\n");
        STR("  "), STR(argv[0]), STR(" "),
            STR("skip [count]           Skips [count] 16-byte blocks\n");
        STR("  "), STR(argv[0]), STR(" "),
            STR("stop                   Stops SD operations and "
                    "de-inits everything\n");
        STR("  "), STR(argv[0]), STR(" "),
            STR("ctrl                   Does nothing\n");
    }


    return 0;
}


int shell_func_load(struct shell_env *env, int argc, char **argv) {
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
        STR("You can also specify a config_block partition name as the offset, "
                "such as img1 or krnA, and omit the [bytes] parameter.\n");
    }

    return 0;
}

int shell_func_call(struct shell_env *env, int argc, char **argv) {
    if(argc > 1) {
        int (*t)() = (int (*)())simple_strtoul(argv[1], NULL, 0);
        t();
    }
    else
        STR("Usage: call [address]");
    return 0;
}

int shell_func_jump(struct shell_env *env, int argc, char **argv) {
    STR("jump not yet implemented");
    return 0;
}

int shell_func_dump(struct shell_env *env, int argc, char **argv) {
    if(argc > 1) {
        int count = 1;
        int offset = simple_strtoul(argv[1], NULL, 0);
        if(argc > 2)
            count = simple_strtoul(argv[2], NULL, 0);
        HEXDUMP((unsigned char *)offset, count);
    }
    else {
        STR("  "), STR(argv[0]), STR(" "),
            STR("[offset] [<count>]  Dumps [count] bytes of memory at [offset].  Count is 1 if unspecified.\n");
    }
    return 0;
}


int shell_func_linux(struct shell_env *env, int argc, char **argv) {
    char *cmdline       = DEFAULT_LINUX_CMDLINE;
    char *offset        = NULL;
    char *initrd_offset = NULL;
    int initrd_size     = 0;
    if(argc > 1)
        offset = (unsigned char *)simple_strtoul(argv[1], NULL, 0);
    if(argc > 2)
        cmdline = argv[2];


    if(offset) {
        STR("Going to run linux at offset "), HEX((long)offset), STR(" with cmdline "), STR(cmdline), STR("\n");

        // If an initrd was specified, include that.
        if(argc > 4) {
            initrd_offset = simple_strtoul(argv[3], NULL, 0);
            initrd_size   = simple_strtoul(argv[4], NULL, 0);
        }

        // Jump to the Linux kernel.
        run_linux(offset, cmdline, initrd_offset, initrd_size);
        STR("Returned from run_linux.  How?!");
        return 0;
    }

    else {
        STR("  "), STR(argv[0]), STR(" "),
            STR("[offset] [<cmdline>]  Runs the linux kernel at offset [offset], optinally with the arguments [<cmdline>]\n");
        STR("  Note: The <cmdline> argument must be quoted.\n");
        return 0;
    }
}


int shell_func_lcd(struct shell_env *env, int argc, char **argv) {
    if(argc > 1 && !strcmp(argv[1], "init")) {
        unsigned char *lcd_fb_offset = 0x43e00000;
        if(argc > 2)
            lcd_fb_offset = simple_strtoul(argv[2], NULL, 0);

        // Set up pin banks to enable LCD function.
        HW_PINCTRL_MUXSEL2_WR(0x00000000);
        HW_PINCTRL_MUXSEL3_WR(0x3fa00fc0);
        udelay(50);



        // Un-gate the PIX clock and set it to the "correct" frequency.
        // Empirical tests indicate that this frequency reduces
        // flicker entirely.
        HW_CLKCTRL_PIX_WR(0x00000002);
        udelay(50);


        // Zero out the framebuffer.
        memset(lcd_fb_offset, 0, FRAMEBUFFER_BYTES);

        // Set the controller to the correct protocol.
        HW_LCDIF_CTRL_WR(0x000b0820);
        HW_LCDIF_NEXT_BUF_WR(lcd_fb_offset);
        HW_LCDIF_CUR_BUF_WR(lcd_fb_offset);
        udelay(50);


        // Slam in the configuration registers gathered from Linux.
        HW_LCDIF_CTRL1_WR(0x010f0701);
        udelay(50);


        // Set up timing.
        HW_LCDIF_TRANSFER_COUNT_WR(0x00f00140);
        HW_LCDIF_TIMING_WR(0x01010101);
        HW_LCDIF_VDCTRL0_WR(0x1c300003);
        HW_LCDIF_VDCTRL1_WR(0x000000f9);
        HW_LCDIF_VDCTRL2_WR(0x28000179);
        HW_LCDIF_VDCTRL3_WR(0x00340006);
        HW_LCDIF_VDCTRL4_WR(0x00040140);
        udelay(50);


        // Enable the LCD controller.
        HW_LCDIF_CTRL_WR(0x000b0821);



        // While we're at it, set the backlight to fullbright.
        // It lives in PWM, bank 1, pin 28.  Set it to GPIO.
        HW_PINCTRL_DOUT1_SET(0x10000000);
        HW_PINCTRL_DOE1_SET(0x10000000);
        HW_PINCTRL_DOUT1_SET(0x10000000);
        udelay(50);

    }

    else if(argc > 2 && !strcmp(argv[1], "fb")) {
        HW_LCDIF_NEXT_BUF_WR(simple_strtoul(argv[2], NULL, 0));
    }

    else if(argc > 1 && !strcmp(argv[1], "clear")) {
        memset((unsigned long *)HW_LCDIF_CUR_BUF_RD(), 0, FRAMEBUFFER_BYTES);
    }

    else {
        STR(argv[0]), STR(" usage:\n");
        STR("  "), STR(argv[0]), STR(" "),
            STR("init               Initialize video.\n");
        STR("  "), STR(argv[0]), STR(" "),
            STR("fb <address>       Re-target video framebuffer to address [address]\n");
        STR("  "), STR(argv[0]), STR(" "),
            STR("clear              Clears the screen\n");
    }

    return 0;
}



int shell_func_shell(struct shell_env *env, int argc, char **argv) {
    int go_interactive  = 0;
    int wait_time       = DEFAULT_WAIT_TIME;
    char line[512];

    // Print out the greeting banner.
    STR("\n\n\nChumby bootloader console " BOOTLOADER_VERSION "\n");
    STR("Booting in "),INT(wait_time),STR(" seconds...\n");


    // When wait_time reaches 0, we've timed out.  Give the user that much
    // time to enter the shell, otherwise we'll just continue.
    go_interactive = shell_wait_prompt("\rPress any key to enter shell... ",
                                       wait_time);
    STR("\n");

    env->should_exit = 0;

    if(go_interactive)
        STR("Bootloader shell.  Type \"help\" for help.\n");


    // Continue reading in lines as long as we're interactive.
    while(go_interactive && !env->should_exit) {
        STR(PROMPT);

        // Read a whole line in.  This function is guaranteed to return a
        // valid, null-terminated string.
        shell_read_line(line, sizeof(line));
        execute_command(line);
    }

    env->should_exit = 0;

    return 0;
}

int shell_func_sleep(struct shell_env *env, int argc, char **argv) {
    int sleep_time = 0;
    if(argc > 1)
        sleep_time = simple_strtoul(argv[1], NULL, 0);
    sleep(sleep_time);
    return 0;
}

int shell_func_msleep(struct shell_env *env, int argc, char **argv) {
    int sleep_time = 0;
    if(argc > 1)
        sleep_time = simple_strtoul(argv[1], NULL, 0);
    msleep(sleep_time);
    return 0;
}

int shell_func_usleep(struct shell_env *env, int argc, char **argv) {
    int sleep_time = 0;
    if(argc > 1)
        sleep_time = simple_strtoul(argv[1], NULL, 0);
    usleep(sleep_time);
    return 0;
}

static char hex[] = "0123456789abcdef";

// str must be at least 11 bytes long
void str_puthex(u32 c, char *str) {
  int i, j;
    str[0] = '0'; str[1] = 'x';
    for(i=7, j = 0; i>=0; i--, j++)
      str[2+j] = hex[(c>>(4*i))&0x0f];
    
    str[10] = '\n';
    str[11] = '\0';
}

/*
int shell_func_burnkeys(struct shell_env *env, int argc, char **argv) {

  int address, data;
  unsigned int key[4];
  char cmdline[100];
  char base[] = "otp -w 0x";
  unsigned int otpdat;

  // put some checks in to make sure we're allowed to do this
  otpdat = HW_RTC_PERSISTENT1_RD();
  if( (otpdat & 0x08000000) == 0 ) {
    STR("Skipping OTP burn.\n");
    return 0;
  }
  
  HW_OCOTP_CTRL_SET(0x3000); // open and reload
  while(HW_OCOTP_CTRL_RD() & 0x100) // wait for writes to finish
    ;
  
  otpdat = HW_OCOTP_LOCK_RD();
  if( (otpdat & 0x30 != 0x00) ) {
    STR("OTP status already locked, disallowing re-burn.\n");
    return 0;
  }

  // now read the keys
  key[3] = HW_RTC_PERSISTENT5_RD();
  key[2] = HW_RTC_PERSISTENT4_RD();
  key[1] = HW_RTC_PERSISTENT3_RD();
  key[0] = HW_RTC_PERSISTENT2_RD();

  HW_RTC_PERSISTENT1_CLR(0x08000000); // clear the OTP flag in case we reboot
  HW_RTC_PERSISTENT5_WR(0);  // zero it out just in case it sticks around
  HW_RTC_PERSISTENT4_WR(0);
  HW_RTC_PERSISTENT3_WR(0);
  HW_RTC_PERSISTENT2_WR(0);

#if OTP_CONSOLE_VERIFY
  STR( "AES key: \n" );
  STR( "3: " ), HEX(key[3]), STR("\n");
  STR( "2: " ), HEX(key[2]), STR("\n");
  STR( "1: " ), HEX(key[1]), STR("\n");
  STR( "0: " ), HEX(key[0]), STR("\n");
#endif

  // 0x8002c000 is OCOTP
  strcpy(cmdline, base);
  cmdline[9] = '7';
  cmdline[10] = ' ';
  str_puthex(key[3], &(cmdline[11]));
#if OTP_TESTING
  shell_write_str(cmdline);
#else
  execute_command(cmdline);
#endif
  while(HW_OCOTP_CTRL_RD() & 0x100) // wait for writes to finish
    ;

  strcpy(cmdline, base);
  cmdline[9] = '6';
  cmdline[10] = ' ';
  str_puthex(key[2], &(cmdline[11]));
#if OTP_TESTING
  shell_write_str(cmdline);
#else
  execute_command(cmdline);
#endif
  while(HW_OCOTP_CTRL_RD() & 0x100) // wait for writes to finish
    ;

  strcpy(cmdline, base);
  cmdline[9] = '5';
  cmdline[10] = ' ';
  str_puthex(key[1], &(cmdline[11]));
#if OTP_TESTING
  shell_write_str(cmdline);
#else
  execute_command(cmdline);
#endif
  while(HW_OCOTP_CTRL_RD() & 0x100) // wait for writes to finish
    ;

  strcpy(cmdline, base);
  cmdline[9] = '4';
  cmdline[10] = ' ';
  str_puthex(key[0], &(cmdline[11]));
#if OTP_TESTING
  shell_write_str(cmdline);
#else
  execute_command(cmdline);
#endif
  while(HW_OCOTP_CTRL_RD() & 0x100) // wait for writes to finish
    ;

  // reload the shadow registers
  HW_OCOTP_CTRL_SET(0x3000); // open and reload
  while(HW_OCOTP_CTRL_RD() & 0x100) // wait for writes to finish
    ;

#if OTP_CONSOLE_VERIFY
#warning "OTP_CONSOLE_VERIFY is just for verification, should not be enabled for production.\n";
  STR("Programmed OTP values (this is for debug only, TURN OFF FOR PRODUCTION!):\n");
  otpdat = HW_OCOTP_CRYPTO3_RD();
  STR("3: "), HEX(otpdat), STR("\n");
  otpdat = HW_OCOTP_CRYPTO2_RD();
  STR("2: "), HEX(otpdat), STR("\n");
  otpdat = HW_OCOTP_CRYPTO1_RD();
  STR("1: "), HEX(otpdat), STR("\n");
  otpdat = HW_OCOTP_CRYPTO0_RD();
  STR("0: "), HEX(otpdat), STR("\n");
#endif

#if OTP_LOCK
  HW_OCOTP_CTRL_SET(0x3000); // open and reload
  while(HW_OCOTP_CTRL_RD() & 0x100) // wait for writes to finish
    ;
  
  otpdat = HW_OCOTP_LOCK_RD();
  
  // program the lock bits on the DCP
  otpdat |= 0x00600030;
  
  STR("Programming OTP 0x10 (lock) with "), HEX(otpdat), STR("\n");
  
  strcpy(cmdline, base);
  cmdline[9] = '1';
  cmdline[10] = '0';
  cmdline[11] = ' ';
  str_puthex(otpdat, &(cmdline[12]));
#if OTP_TESTING
  shell_write_str(cmdline);
#else
  execute_command(cmdline);
#endif
  while(HW_OCOTP_CTRL_RD() & 0x100) // wait for writes to finish
    ;
#else
#warning "OTP_LOCK is not set, this build is not suitable for production.\n";
  STR("WARNING WARNING WARNING\n");
  STR("OTP is not locked!\n");
#endif

  HW_OCOTP_CTRL_SET(0x3000); // open and reload
  while(HW_OCOTP_CTRL_RD() & 0x100) // wait for writes to finish
    ;
  
#if OTP_CONSOLE_VERIFY
  STR("Locked OTP values (should be 0xBADABADA(?)):\n");
  otpdat = HW_OCOTP_CRYPTO3_RD();
  STR("3: "), HEX(otpdat), STR("\n");
  otpdat = HW_OCOTP_CRYPTO2_RD();
  STR("2: "), HEX(otpdat), STR("\n");
  otpdat = HW_OCOTP_CRYPTO1_RD();
  STR("1: "), HEX(otpdat), STR("\n");
  otpdat = HW_OCOTP_CRYPTO0_RD();
  STR("0: "), HEX(otpdat), STR("\n");
#endif
  
  STR("Rebooting...");
  sys_reboot();

}
*/




static struct shell_command cmds[] = {
    {
        .name   = "unknown",
        .help   = NULL,
        .func   = shell_func_unknown,
    },
    {
        .name   = "help",
        .help   = NULL,
        .func   = shell_func_help,
    },
    {
        .name   = "exit",
        .help   = NULL,
        .func   = shell_func_exit,
    },
    {
        .name   = "print",
        .help   = NULL,
        .func   = shell_func_print,
    },
    {
        .name   = "regutil",
        .help   = NULL,
        .func   = shell_func_regutil,
    },
    {
        .name   = "otp",
        .help   = NULL,
        .func   = shell_func_otp,
    },
    {
        .name   = "reboot",
        .help   = NULL,
        .func   = shell_func_reboot,
    },
    {
        .name   = "sd",
        .help   = NULL,
        .func   = shell_func_sd,
    },
    {
        .name   = "lcd",
        .help   = NULL,
        .func   = shell_func_lcd,
    },
    {
        .name   = "load",
        .help   = NULL,
        .func   = shell_func_load,
    },
    {
        .name   = "call",
        .func   = shell_func_call,
    },
    {
        .name   = "jump",
        .func   = shell_func_jump,
    },
    {
        .name   = "dump",
        .func   = shell_func_dump,
    },
    {
        .name   = "linux",
        .func   = shell_func_linux,
    },
    {
        .name   = "shell",
        .func   = shell_func_shell,
    },
    {
        .name   = "sleep",
        .func   = shell_func_sleep,
    },
    {
        .name   = "msleep",
        .func   = shell_func_msleep,
    },
    {
        .name   = "usleep",
        .func   = shell_func_usleep,
    },
    /*
    {
        .name   = "burnkeys",
        .func   = shell_func_burnkeys,
    },
    */

    // NULL-terminate the array.
    {
        .name   = NULL,
        .help   = NULL,
        .func   = NULL,
    }
};

struct shell_command *shell_funcs_init() {
    struct shell_command *cmd_fixup = cmds;

    // Fixup the cmd array by adding the base_address value to each pointer.
    while(cmd_fixup->name) {
        cmd_fixup->name += base_address;
        cmd_fixup->func += base_address;
        if(cmd_fixup->help)
            cmd_fixup->help += base_address;
        cmd_fixup++;
    }
    return cmds;
}
