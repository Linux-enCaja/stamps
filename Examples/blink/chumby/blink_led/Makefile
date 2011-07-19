CURRENTDIR	= .
SRCDIR		= $(CURRENTDIR)/src
OUTPUTDIR	= $(CURRENTDIR)/output

INCLUDEDIR	= $(CURRENTDIR)/include

# Linker script 
BASE_ADDR	?= 0x00002000
BOOT_LAYOUT_IN	= $(SRCDIR)/blink_led.ld.in
BOOT_LAYOUT_OUT	= $(OUTPUTDIR)/blink_led.ld


# Output ELF image
BLINK_LED_ELF	= $(OUTPUTDIR)/blink_led.elf

# Output binary image
BLINK_LED_BIN	= $(OUTPUTDIR)/blink_led.bin

CROSS_COMPILE ?= arm-linux-

AS	= $(CROSS_COMPILE)as
CC	= $(CROSS_COMPILE)gcc
LD	= $(CROSS_COMPILE)ld
CPP	= $(CROSS_COMPILE)cpp
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP	= $(CROSS_COMPILE)objdump

LIBGCCDIR = $(dir $(shell $(CC) -print-libgcc-file-name))
CFLAGS 	= -O0 -g -Wall -I$(INCLUDEDIR) -I$(LINUX_DIR)/include -nostdinc -fno-builtin
LDFLAGS = -static -nostdlib -T $(BOOT_LAYOUT_OUT) -L$(LIBGCCDIR) -lgcc


# Generic code
SRC_OBJS = entry.o  main.o


BLINK_LED_OBJS = $(addprefix $(SRCDIR)/, $(SRC_OBJS))

# Default goal
.PHONY: all
all: build



#
# Define an implicit rule for assembler files
# to run them through C preprocessor
#
%.o: %.S
	$(CC) -c $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# Make targets
#
.PHONY: build build_prep clean

build: build_prep $(BLINK_LED_BIN)

build_prep:
	mkdir -p $(OUTPUTDIR)

clean:
	@echo Cleaning...
	@echo Files:
	rm -rf $(BLINK_LED_OBJS) $(BOOT_LAYOUT_OUT)
	@echo Build output:
	rm -rf $(OUTPUTDIR)

##
## Rules to build linux_prep image
## 
#$(CMDLINES_STRIP): $(CMDLINES)
#	$(call strip_cmdlines_file)

$(BLINK_LED_BIN): $(BLINK_LED_ELF)
	$(OBJCOPY) -R -S -O binary -R .note -R .note.gnu.build-id -R .comment $< $@

$(BLINK_LED_ELF): $(BLINK_LED_OBJS) $(BOOT_LAYOUT_OUT)
	$(LD) -o $@ $(BLINK_LED_OBJS) $(LDFLAGS)
	@nm -n $@ > $@.map

$(BOOT_LAYOUT_OUT): $(BOOT_LAYOUT_IN)
	$(CPP) -P -DBASE_ADDR=$(BASE_ADDR) -o $@ $<

sb:
	./elftosb2 -V -c blink_led.db \
            -o $(OUTPUTDIR)/blink_led_sb.bin
	dd if=/dev/zero of=sd_blink.raw bs=512 count=4
	dd if=output/blink_led_sb.bin of=sd_blink.raw ibs=512 seek=4 conv=sync,notrunc
	sudo dd if=sd_blink.raw of=/dev/sdb1

