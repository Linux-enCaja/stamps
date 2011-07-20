#ifndef __SD_H__
#define __SD_H__

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

#endif //__SD_H__
