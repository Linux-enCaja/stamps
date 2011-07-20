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
#include "serial.h"
#include "otp.h"
#include "utils.h"
#include "sd.h"
#include "ts.h"
#include "shell.h"
#include "shell_funcs.h"
#include "sd_config_block.h"

int partition = 0;
int recovery  = 0;
struct shell_env env;
struct shell_command *shell_commands;


#define LCD_FB_OFFSET "0x44844000"

// Commands may be prefixed by a three-character code:
//  The first character is !, indicating it's a special command.
//  The second character is one of the following:
//      S   - Sets a variable
//      I   - Executes the command if the variable is nonzero
//      N   - Executes the command if the variable is zero
// The thid character is a letter in the range [A-Z], representing the
// variable to use.

static char *command_lines[] = {

// The default command line does basic initialization.  It is common to all
// boot modes.



    // Prompt the user to enter the shell
    "shell\n"

    // Run the watchdogtimer, set to reboot the device in 20 seconds.
    "regutil -w HW_RTC_WATCHDOG 0x00004e20\n"
    "regutil -w HW_RTC_CTRL_SET 0x00000010\n"

    // Prioritize screen DMA over other DMA.  This fixes video tearing issues.
    "regutil -w HW_EMI_CTRL 0x1c444040\n"

    // Disable brownout detectors, for the same reason.
    "regutil -w HW_POWER_BATTMONITOR 0x01ec0414\n"
    "regutil -w HW_POWER_5VCTRL_CLR 0x00000080\n"
    "regutil -w HW_POWER_MINPWR_SET 0x00001000\n"


    // Enable USB current.  Otherwise the device will be unstable when it
    // goes to power up the battery.
    "regutil -w HW_PINCTRL_MUXSEL1_SET 0x03300000\n"
    "regutil -w HW_PINCTRL_DOUT0_CLR 0x24000000\n"
    "regutil -w HW_PINCTRL_DOE0_SET 0x24000000\n"
    "regutil -w HW_PINCTRL_DOUT0_CLR 0x24000000\n"
    "regutil -w HW_PINCTRL_DOUT0_SET 0x20000000\n"
    "regutil -w HW_PINCTRL_DOUT0_SET 0x04000000\n"

    "",
////////////////////////////////////////////////

// The first numbered command line loads rfsA as the main boot partition.
    // Load the correct kernel.
    "load krnA 0x40008000\n"

    // Now that the kernel is loaded, draw the new splash screen.
//    "load img2 " LCD_FB_OFFSET "\n"

    // Set up the Linux tags, pass the command-line arguments, and run the
    // Linux kernel, located at address 0x40008000.
    "linux 0x40008000 \"console=ttyAM0,115200 init=/linuxrc root=/dev/mmcblk0p2 rootfstype=ext3 ro rootwait lcd_panel=lms350 ssp1=mmc line=1 sysrq_always_enabled rotary=1\"\n",

////////////////////////////////////////////////

// The next numbered command line loads rfsA as the recovery partition.
    // Load the correct kernel.
    "load krnA 0x40008000\n"

    // Now that the kernel is loaded, draw the new splash screen.
//    "load img4 " LCD_FB_OFFSET "\n"

    // Set up the Linux tags, pass the command-line arguments, and run the
    // Linux kernel, located at address 0x40008000.
    "linux 0x40008000 \"console=ttyAM0,115200 init=/linuxrc root=/dev/mmcblk0p2 rootfstype=ext3 ro rootwait lcd_panel=lms350 ssp1=mmc line=1 sysrq_always_enabled rotary=1 partition=recovery\"\n",

////////////////////////////////////////////////

// This command line loads rfsB as the primary partition.
    // Load the correct kernel.
    "load krnB 0x40008000\n"

    // Now that the kernel is loaded, draw the new splash screen.
//    "load img2 " LCD_FB_OFFSET "\n"

    // Set up the Linux tags, pass the command-line arguments, and run the
    // Linux kernel, located at address 0x40008000.
    "linux 0x40008000 \"console=ttyAM0,115200 init=/linuxrc root=/dev/mmcblk0p3 rootfstype=ext3 ro rootwait lcd_panel=lms350 ssp1=mmc line=1 sysrq_always_enabled rotary=1\"\n",

////////////////////////////////////////////////

// Finally, this command line loads rfsB as the recovery partition.
    // Load the correct kernel.
    "load krnB 0x40008000\n"

    // Now that the kernel is loaded, draw the new splash screen.
//    "load img4 " LCD_FB_OFFSET "\n"

    // Set up the Linux tags, pass the command-line arguments, and run the
    // Linux kernel, located at address 0x40008000.
    "linux 0x40008000 \"console=ttyAM0,115200 init=/linuxrc root=/dev/mmcblk0p3 rootfstype=ext3 ro rootwait lcd_panel=lms350 ssp1=mmc line=1 sysrq_always_enabled rotary=1 partition=recovery\"\n",

////////////////////////////////////////////////

// This will be used if and when we have an rfsC
    // Load the correct kernel.
    "load krnC 0x40008000\n"

    // Now that the kernel is loaded, draw the new splash screen.
    "load img6 " LCD_FB_OFFSET "\n"

    "load init 0x00\n"

    // Set up the Linux tags, pass the command-line arguments, and run the
    // Linux kernel, located at address 0x40008000.
    "linux 0x40008000 \"console=ttyAM0,115200 root=/dev/ram0 ro rootwait lcd_panel=lms350 ssp1=mmc line=1 sysrq_always_enabled rotary=1 failsafe=1\"\n",
};



int shell_cmd_interpreter(int argc, char **argv, int function);


struct reg_info {
    char *name;
    int offset;
};



int shell_write_str(char *str) {
     serial_puts(str);
     return 0;
}

int shell_write_hex(int x) {
    serial_puthex(x);
    return 0;
}

int shell_write_chr(char c) {
    serial_putc(c);
    return 0;
}

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


#define HEXDIGITS "0123456789abcdef"
int shell_write_hexdump(unsigned char *c, int size) {
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








static int tab_complete(char *buffer, int length, int size) {
    CHR(0x07);
    return size;
}



int shell_read_line(char *buffer, int length) {
    int size = 0;
    int c;
    int should_exit = 0;
    if(length <= 0)
        return 0;
    
    while(!should_exit && size < length-1) {
        c = serial_getc();
        //HEX(c);
        switch(c) {

            // "Delete", on most keyboards.
            case '\b':
            case 0x7f:
                if(size > 0) {
                    size--;
                    serial_putc(0x08);
                    serial_putc(' ');
                    serial_putc(0x08);
                }
                break;

            case '\r':
            case '\n':
                should_exit = 1;
                serial_putc(c);
                serial_putc('\n');
                break;

            // Control-U
            case 0x15:
                while(size) {
                    serial_putc(0x08);
                    serial_putc(' ');
                    serial_putc(0x08);
                    size--;
                }
                break;

            // Tab-completion.
            case '\t':
                size = tab_complete(buffer, length, size);
                break;


            // What happens when someone hits control-D.
            case EOF:
                if(size==0) {
                    execute_command("exit");
                    should_exit=1;
                }
                break;

            default:
                if(isprint(c)) {
                    buffer[size++] = c;
                    serial_putc(c);
                }
                break;
        }
    }


    // Null-terminate the string, as the size pointer has already been
    // advanced.
    buffer[size] = '\0';
    return size;
}


// Processes *line by splitting it along what we consider spaces (taking
// into account quoting and so forth).
// Return the number of args found in the upper short, and the decoded
// command in the lower word.
// Remove comments, which begin with a "#" sign.
int shell_process_args(char *line) {
    int argc = 1;
    int in_quote = 0;
    char *ptr = line;
    while(*ptr) {
        if(in_quote && *ptr=='"') {
            in_quote = 0;
            *ptr = ' ';
        }

        else if(!in_quote && *ptr == '#')
            *ptr = '\0';

        if(*ptr == ' ' && !in_quote) {
            while(*ptr && *ptr == ' ') {
                *ptr = '\0';
                ptr++;
            }
            // Comments indicate the processing of the line should cease.
            if(*ptr == '#')
                *ptr = '\0';
            if(*ptr)
                argc++;
            if(*ptr=='"') {
                in_quote=1;
                *ptr='\0';
            }
        }

        ptr++;
    }

    return argc;
}

char **shell_line_to_argv(int argc, char *line) {
    static char *argv[80];
    int i;
    for(i=0; i<=argc; i++) {
        argv[i] = line;
        while(*line)
            line++;
        while(!*line)
            line++;
    }
    return argv;
}




int shell_wait_prompt(char *line, int wait_time) {
    int start_time;

    start_time = HW_RTC_MILLISECONDS_RD()-1000;
    wait_time++;
    while(wait_time > 0) {

        // Decrement the wait counter.
        if(HW_RTC_MILLISECONDS_RD()-start_time > 1000) {
            wait_time--;
            start_time = HW_RTC_MILLISECONDS_RD();
            STR("\rPress any key to enter shell... "), INT(wait_time);
        }

        // If there are characters in the buffer, pull them off and enter
        // the shell.
        if(serial_tstc()) {
            return 1;
        }
    }

    return 0;
}




int execute_command(char *line) {
    int argc;
    char **argv;


    // Process the line, generating the argc and argv, as well as the
    // command to be executed.
    argc = shell_process_args(line);

    argv = shell_line_to_argv(argc, line);
//    STR("Command: "), STR(argv[0]), STR("  Argc: "), INT(argc), STR("\n");

    // 
    return shell_cmd_interpreter(argc, argv, 0);
}


#define SHELL_COMMAND(x)                                                    \
    if(function == 0 && !strcmp(argv[0], #x))                               \
        found = 1, result = shell_func_##x (&env, argc, argv);              \
    if(function == 1 && (!argv || !strncmp(argv[0], #x, strlen(argv[0]))))  \
        result++, STR(#x), STR("\n");                                       

// The command interpreter handles translation of strings to commands.
// The function depends on the function you pass:
//      0 - Executes the command passed in as argv[0].
//      1 - Finds all commands that begin with argv[0], and prints them.
//          Note that as a special case, argv can be NULL to print all options.
int shell_cmd_interpreter(int argc, char **argv, int function) {
    int result = 0, found = 0;
    int position;

    if(function)
        return 0;

    // Look through all known commands for a matching command.
    // If one is found, run that command.
    for(position=0; shell_commands[position].name; position++) {
        if(!strcmp(argv[0], shell_commands[position].name)) {
            result = shell_commands[position].func(&env, argc, argv);
            found  = 1;
            break;
        }
    }

    // If a command wasn't found, print the generic "Unknown!" message.
    if(!found)
        shell_commands[0].func(&env, argc, argv);

    return result;
}


extern char *sd_pmem;
extern struct config_area *sd_cfg;

int execute_script(char *shell_script) {
    env.should_exit = 0;
    while(!env.should_exit) {
        char *line_start = shell_script;

        // Look for either the linefeed or the end-of-string marker on the
        // default command line.  If we don't find it, advance one
        // character and try again.
        while(*shell_script != '\n' && *shell_script != '\0')
            shell_script++;

        // If we hit the NULL character on the default command line, we're
        // done.
        if(!*shell_script)
            env.should_exit = 1;

        // Null-terminate the current command.
        *shell_script = '\0';
        STR(PROMPT), STR(line_start), STR("\n");


        // This function executes the command, and returns whether or not
        // we should continue.  It returns true if we should continue, and
        // false if we should quit.
        execute_command(line_start);
        shell_script++;
    }
    return env.should_exit;
}

extern struct config_area *sd_cfg;

u32 enter_shell() {
    char sd_data[PMEM_SIZE];
    int screen_pressed;

    // Allocate some data on the stack for the pmem structure of the sd
    // card routines.
    sd_pmem = sd_data;

    // Initialize the shell environment.
    env.should_exit = 0;
    env.shell_commands = shell_funcs_init();
    shell_commands = env.shell_commands;

    // Set up writing (AND READING!) of the serial port.
    //shell_init();


    // Set up the touchscreen.
    ts_init();
    screen_pressed = ts_pressed();
    STR("Screen pressed: "), INT(screen_pressed), STR("\n");


    // Draw the appropriate splash image to the screen.
    execute_command("lcd init " LCD_FB_OFFSET);
    execute_command("lcd fb " LCD_FB_OFFSET);
    if(!screen_pressed)
        execute_command("load img1 " LCD_FB_OFFSET);
    else
        execute_command("load img3 " LCD_FB_OFFSET);



    // Load up the default shell string, which includes a lot of common
    // init stuff.
    execute_script(command_lines[0]);


    // Determine whether to boot to rfsA or rfsB.
    partition = !!sd_cfg->active_index;
    recovery  = screen_pressed && ts_pressed();


    // Draw the stage-two splash screen.
    if(!recovery)
        execute_command("load img2 " LCD_FB_OFFSET);
    else
        execute_command("load img5 " LCD_FB_OFFSET);


    // Now, figure out whether we'll be running rfsA or rfsB, and whether
    // we'll be going to recovery mode or not.
    if(     partition==0 && recovery==0)
        execute_script(command_lines[1]);
    else if(partition==1 && recovery==1)
        execute_script(command_lines[2]);
    else if(partition==1 && recovery==0)
        execute_script(command_lines[3]);
    else if(partition==0 && recovery==1)
        execute_script(command_lines[4]);
    else {
        STR("Don't know which command line to use!  Going to recovery mode.\n");
        execute_script(command_lines[5]);
    }


    STR("Warning: Exiting from shell.  Probably entering into nowhere land.\n");

    return 0;
}

