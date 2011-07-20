CURRENTDIR	= 
SRCDIR		= ./src
OUTPUTDIR	= ./output
INCLUDEDIR	= ./include

# Linker script 
BASE_ADDR	?= 0x00002000
BOOT_LAYOUT_IN	= $(SRCDIR)/link_script.ld.in
BOOT_LAYOUT_OUT	= $(SRCDIR)/link_script.ld
TARGET          = sdram_test

# Output ELF image
BLINK_LED_ELF	= $(OUTPUTDIR)/$(TARGET).elf

# Output binary image
BLINK_LED_BIN	= $(OUTPUTDIR)/$(TARGET).bin

CROSS_COMPILE ?= arm-none-linux-gnueabi-

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


VPATH = %.c %.S $(SRCDIR)

OBJS = main.o src/entry.o hardware_init.o

# Default goal
.PHONY: all
all: build



#
# Define an implicit rule for assembler files
# to run them through C preprocessor
#
$(SRCDIR)/%.o: %.S
	$(CC) -c $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# Make targets
#
.PHONY: build build_prep clean

build: build_prep $(TARGET).bin

build_prep:
	mkdir -p $(OUTPUTDIR)
	chmod +x ./elftosb2
clean:
	@echo Cleaning...
	@echo Files:
	rm -rf $(OBJS) $(BOOT_LAYOUT_OUT) $(TARGET).bin $(TARGET).elf
	@echo Build output:
	rm -rf $(OUTPUTDIR) *.map $(TARGET).raw

##
## Rules to build linux_prep image


$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -R -S -O binary -R .note -R .note.gnu.build-id -R .comment $(OUTPUTDIR)/$< $(OUTPUTDIR)/$@

$(TARGET).elf: $(OBJS) $(BOOT_LAYOUT_OUT)
	$(LD) -o $(OUTPUTDIR)/$@ $(OBJS) $(LDFLAGS)
	@nm -n $(OUTPUTDIR)/$@ > $(OUTPUTDIR)/$@.map

$(BOOT_LAYOUT_OUT): $(BOOT_LAYOUT_IN)
	$(CPP) -P -DBASE_ADDR=$(BASE_ADDR) -o $@ $<

sb: build
	./elftosb2 -V -c $(SRCDIR)/$(TARGET).db -o $(OUTPUTDIR)/$(TARGET)_sb.bin
	dd if=/dev/zero of=$(OUTPUTDIR)/$(TARGET).raw bs=512 count=4
	dd if=$(OUTPUTDIR)/$(TARGET)_sb.bin of=$(OUTPUTDIR)/$(TARGET).raw ibs=512 seek=4 conv=sync,notrunc
	sudo dd if=$(OUTPUTDIR)/$(TARGET).raw of=/dev/mmcblk0p1

