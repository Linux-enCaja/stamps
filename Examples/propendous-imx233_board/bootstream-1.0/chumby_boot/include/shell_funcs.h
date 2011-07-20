#ifndef __SHELL_FUNCS_H__
#define __SHELL_FUNCS_H__

struct shell_env;

struct shell_command {
    char *name;
    char *help;
    int (*func)(struct shell_env *env, int argc, char **argv);
};


struct shell_env {
    int should_exit;
    struct shell_command *shell_commands;
};



// Load addresses can be handy for finding places to load code.
#define LINUX_LOAD_ADDRESS  0x40008000
#define LINUX_LOAD_SIZE     0x00400000
#define LINUX_k1_LOAD_SOURCE 0x00100000
#define LINUX_k2_LOAD_SOURCE 0x00500000
#define LINUX_k3_LOAD_SOURCE 0x00900000
#define INITRD_LOAD_ADDRESS LINUX_LOAD_ADDRESS+LINUX_LOAD_SIZE
#define INITRD_LOAD_SIZE    0x00400000
#define INITRD_LOAD_SOURCE  0x00b00000
#define CHUMBY_LOAD_ADDRESS INITRD_LOAD_ADDRESS+INITRD_LOAD_SIZE
#define CHUMBY_LOAD_SIZE    0x000f7800
#define CHUMBY_LOAD_SOURCE  0x00008800


struct shell_command *shell_funcs_init();
/*
int shell_func_help(char *env, int argc, char **argv);
int shell_func_unknown(char *env, int argc, char **argv);
int shell_func_exit(char *env, int argc, char **argv);
int shell_func_print(char *env, int argc, char **argv);
int shell_func_regutil(char *env, int argc, char **argv);
int shell_func_otp(char *env, int argc, char **argv);
int shell_func_reboot(char *env, int argc, char **argv);
int shell_func_sd(char *env, int argc, char **argv);
int shell_func_lcd(char *env, int argc, char **argv);
int shell_func_load(char *env, int argc, char **argv);
int shell_func_call(char *env, int argc, char **argv);
int shell_func_jump(char *env, int argc, char **argv);
int shell_func_dump(char *env, int argc, char **argv);
int shell_func_linux(char *env, int argc, char **argv);
int shell_func_shell(char *env, int argc, char **argv);
int shell_func_sleep(char *env, int argc, char **argv);
int shell_func_msleep(char *env, int argc, char **argv);
int shell_func_usleep(char *env, int argc, char **argv);
int shell_func_burnkeys(char *env, int argc, char **argv);
*/

#endif //__SHELL_FUNCS_H__
