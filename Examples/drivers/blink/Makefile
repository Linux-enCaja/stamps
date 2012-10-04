EXTRA_CFLAGS += -Wall
CC            = arm-none-eabi-gcc
KERNEL_SRC    = /home/cain/Embedded/iMx233/Linux/imx23-olinuxino/kernel/linux-mainline/
CROSS_COMPILE = arm-none-eabi-

obj-m +=  blinker.o 
all: driver #main

driver:
	make -C $(KERNEL_SRC) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	make -C $(KERNEL_SRC) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) clean
	rm -rf *.o main.o main irq.ko Modules.symvers

main: main.o

PREPROCESS.c = $(CC) $(CFLAGS) $(TARGET_ARCH) -E -Wp,-C,-dD,-dI
%.pp : %.c FORCE
	$(PREPROCESS.c) $< > $@
