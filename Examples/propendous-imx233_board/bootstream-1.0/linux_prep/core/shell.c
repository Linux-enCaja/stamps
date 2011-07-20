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


#define PROMPT "> "
#define BOOTLOADER_VERSION "v1.0"
#define DEFAULT_WAIT_TIME 3

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



#ifdef __APPLE__
#include <stdio.h>
typedef int u32;
int shell_write_hex(int c) {
    printf("0x%08x", c);
    return 0;
}
#define shell_write_chr putchar
#define shell_write_str(x) fputs(x, stdout)
#define serial_putc putchar
#define serial_getc getchar

#else
#include <setup.h>
#include <stmp3xxx.h>
#include <arch/platform.h>
#include "serial.h"
#define EOF 4
#endif


struct reg_info {
    char *name;
    int offset;
};

struct reg_info regs[] = {
    #include "regutil_falconwing.h"
    {0, 0}
};


struct shell_command {
    char *name;
    char *help;
    int (*func)(int argc, char **argv);
};




static int shell_func_help(int argc, char **argv);
static int shell_func_unknown(int argc, char **argv);
static int shell_func_exit(int argc, char **argv);
static int shell_func_print(int argc, char **argv);
static int shell_func_regutil(int argc, char **argv);


static struct shell_command shell_commands[] = {
    {
        .name = "error",
        .func = shell_func_unknown,
        .help = 0,
    },
    {
        .name = "help",
        .func = shell_func_help,
        .help = 0,
    },
    {
        .name = "exit",
        .func = shell_func_exit,
        .help = 0,
    },
    {
        .name = "print",
        .func = shell_func_print,
        .help = 0,
    },
    {
        .name = "regutil",
        .func = shell_func_regutil,
        .help = 0,
    },
};
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
*/

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

static int strcmp(const char *str1, const char *str2) {
    while(*str1 && *str2) {
        if(*str1 != *str2)
            return 1;
        str1++;
        str2++;
    }

    if(!*str1 && !*str2)
        return 0;
    return 1;
}


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

/***********************************************************************/






#ifndef __APPLE__
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
#endif


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



int shell_read_line(char *buffer, int length) {
    int size = 0;
    int c;
    int should_exit = 0;
    if(length <= 0)
        return 0;
    
    while(!should_exit && size < length-1) {
        c = serial_getc();
        serial_putc(c);
        switch(c) {
            case '\b':
                size--;
                break;
            case '\r':
            case '\n':
                should_exit = 1;
                break;
            case EOF:
                if(size==0) {
                    strcpy(buffer, "exit");
                    size = strlen(buffer);
                    should_exit = 1;
                }
                break;

            default:
                buffer[size++] = c;
                break;
        }
    }

    /*
    // Read the last '\n'.
    c = serial_getc();
    serial_putc(c);
    */

    // Null-terminate the string, as the size pointer has already been
    // advanced.
    buffer[size] = '\0';
    return size;
}








// SHELL_FUNCTIONS

static int shell_func_help(int argc, char **argv) {
    int cmd;
    STR("Available commands:\n");
    for(cmd = 0; cmd < sizeof(shell_commands)/sizeof(struct shell_command); cmd++) {
        STR("    "), STR(shell_commands[cmd].name), STR("\n");
    }
    return 0;
}

static int shell_func_unknown(int argc, char **argv) {
    STR("Unknown command: ");
    STR(argv[0]);
    STR("\n");
    return 0;
}

static int shell_func_exit(int argc, char **argv) {
    return 1;
}

static int shell_func_print(int argc, char **argv) {
    int i;
    for(i=0; i<argc; i++) {
        STR("[");
        STR(argv[i]);
        STR("] ");
    }
    return 0;
}

static int shell_func_regutil(int argc, char **argv) {
    if(argc <= 1) {
        STR("Regutil usage:\n");
        STR("    regutil -r [register-or-offset]          Read an area of RAM\n");
        STR("    regutil -w [register-or-offset] [value]  Write an area of RAM\n");
        STR("    regutil -d                               Dump all registers\n");
    }

    else if(!strcmp(argv[1], "-d")) {
        struct reg_info *reg = regs;
        while(reg->name) {
            HEX(reg->offset), STR(": "), HEX(*((int *)reg->offset));
            STR("  "), STR(reg->name), STR("\n");
            reg++;
        }
    }

    else if(!strcmp(argv[1], "-r")) {
        int read_offset = 0;// strtoul(argv[2], NULL, 0);
        if(!read_offset) {
            struct reg_info *reg = regs;
            while(reg->name) {
                if(!strcmp(reg->name, argv[2])) {
                    read_offset = reg->offset;
                    break;
                }
                reg++;
            }
        }

        if(!read_offset) {
            STR("Unable to locate register "), STR(argv[2]), STR("\n");
        }
        else {
            STR("Value at "), HEX(read_offset), STR(": ");
            HEX(*((int *)read_offset)), STR("\n");
        }
    }

    else if(!strcmp(argv[1], "-w")) {
        int write_offset = 0;
        int write_value = 0;

        write_offset = 0;//strtoul(argv[2], NULL, 0);
        write_value  = 0;//strtoul(argv[3], NULL, 0);

        if(!write_offset) {
            struct reg_info *reg = regs;
            while(reg->name) {
                if(!strcmp(reg->name, argv[2])) {
                    write_offset = reg->offset;
                    break;
                }
                reg++;
            }
        }


        // Actually set the value.
        STR("Setting "), HEX(write_offset), STR(": ");
        HEX(*((int *)write_offset)), STR(" -> ");
        (*((int *)write_offset)) = write_value;
        HEX(*((int *)write_offset)), STR(" ok\n");
    }

    return 0;
}



// Processes *line by splitting it along what we consider spaces (taking
// into account quoting and so forth).
// Return the number of args found in the upper short, and the decoded
// command in the lower word.
int shell_process_args(char *line) {
    int cmd;
    int argc = 1, word = 0;
    int in_quote = 0;
    char *ptr = line;
    while(*ptr) {
        if(in_quote && *ptr=='"') {
            in_quote = 0;
            *ptr = ' ';
        }

        if(*ptr == ' ' && !in_quote) {
            while(*ptr && *ptr == ' ') {
                *ptr = '\0';
                ptr++;
            }
            if(*ptr)
                argc++;
            if(*ptr=='"') {
                in_quote=1;
                *ptr='\0';
            }
        }

        ptr++;
    }

    // Now we've broken everything along word boundaries.  Determine which
    // command the user called.
    for(cmd = 0; cmd < sizeof(shell_commands)/sizeof(struct shell_command); cmd++) {
        if(!strcmp(shell_commands[cmd].name, line)) {
            word=cmd;
            break;
        }
    }

    return (argc<<16) | word;
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


static void screen_paint_logo(int which) {
    return;
}


u32 enter_shell() {
    int should_exit = 1;
    int wait_time = DEFAULT_WAIT_TIME;
    int start_time;



    // Paint the appropriate boot logo.
    screen_paint_logo(0);

    // Set up writing (AND READING!) of the serial port.
    shell_init();



    // Print out the greeting banner.
    STR("Chumby Bootloader " BOOTLOADER_VERSION "\n");


    // When wait_time reaches 0, we've timed out.  Give the user that much
    // time to enter the shell, otherwise we'll just continue.
    start_time = HW_RTC_MILLISECONDS_RD()-1000;
    wait_time++;
    while(wait_time > 0) {

        // Decrement the wait counter.
        if(HW_RTC_MILLISECONDS_RD()-start_time > 1000) {
            wait_time--;
            start_time = HW_RTC_MILLISECONDS_RD();
            STR("\rPress any key to enter boot shell... "), INT(wait_time);
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


    if(!should_exit)
        STR("Bootloader shell.  Type \"help\" for help.\n");


    while(!should_exit) {
        char line[512];
        int  cmd, argc;
        char **argv;
        shell_write_str(PROMPT);

        // Read a whole line in.  This function is guaranteed to return a
        // valid, null-terminated string.
        shell_read_line(line, sizeof(line));



        // Process the line, generating the argc and argv, as well as the
        // command to be executed.
        cmd = shell_process_args(line);
        argc = cmd>>16;
        cmd &= 0xffff;

        argv = shell_line_to_argv(argc, line);



        should_exit = shell_commands[cmd].func(argc, argv);
    }
    return 0;
}

#ifdef __APPLE__
int main(int argc, char **argv) {
    return enter_shell();
}
#endif


