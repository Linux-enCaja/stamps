/*******************************************************************************
 *
 * Filename: irq_main.c
 *  Author:     Carlos Camargo
 *  Created:    June 10, 2010
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ******************************************************************************/

#include "stdio.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  int	       fileNum, bytes;
  unsigned char  buf[40];
  size_t         nbytes;
  ssize_t        bytes_read;

  if(argc != 2){
    fprintf(stderr,"\nUsage: %s enable|disable|read \n",argv[0]);
  }
  
  nbytes = sizeof(int);
  fileNum = open("/dev/irq", O_RDWR);
  if (fileNum < 0) {
    printf(" Unable to open device\n");
    exit(1);
  }
  printf("Device opened successfully \n");

  if(!strcmp(argv[1], "enable"))
    write(fileNum, "Q", 1);
  if(!strcmp(argv[1], "disable"))
    write(fileNum, "S", 1);
  if(!strcmp(argv[1], "read")){
    read(fileNum, buf, nbytes);
    printf("Interrupts = %d \n",  *((int*)(buf)));
  }
  if( (strcmp(argv[1], "read") != 0 ) & (strcmp(argv[1], "disable") != 0) & (strcmp(argv[1], "enable") != 0) )
    fprintf(stderr,"\nUsage: %s enable|disable|read \n",argv[0]);

  close(fileNum);

return (0);
}
