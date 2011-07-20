#ifndef __SHELL_H__
#define __SHELL_H__

#define PROMPT "\nboot> "
#define BOOTLOADER_VERSION "v1.0"
#define DEFAULT_WAIT_TIME 2

#define EOF 4

#define STR shell_write_str
#define INT shell_write_int
#define HEX shell_write_hex
#define CHR shell_write_chr
#define shell_init serial_init
#define HEXDUMP shell_write_hexdump

int shell_cmd_interpreter(int argc, char **argv, int function);

int shell_write_str(char *str);
int shell_write_hex(int x);
int shell_write_chr(char c);
int shell_write_int(int x);
int shell_write_hexdump(unsigned char *c, int size);


int execute_command(char *line);
int shell_wait_prompt(char *line, int wait_time);
int shell_read_line(char *buffer, int length);

extern int base_address;



#endif //__SHELL_H__
