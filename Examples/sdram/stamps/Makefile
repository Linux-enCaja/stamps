OUTPUTDIR	= ./output
SRCDIR		= ./src
INCLUDEDIR	= ./include/
TARGET          = boot_prep

# Linker script
BASE_ADDR	?= 0x00000000
BOOT_LAYOUT	= $(SRCDIR)/link.lds


CROSS_COMPILE ?= arm-none-linux-gnueabi-

AS	= $(CROSS_COMPILE)as
CC	= $(CROSS_COMPILE)gcc
LD	= $(CROSS_COMPILE)ld
CPP	= $(CROSS_COMPILE)cpp
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP	= $(CROSS_COMPILE)objdump

MEM_TYPE ?= MEM_DDR1

CFLAGS 	= -g -Wall -I$(INCLUDEDIR) -I$(INCLUDEDIR)/registers -I./ -O -D$(MEM_TYPE)
LDFLAGS = -static -nostdlib -T $(BOOT_LAYOUT)

VPATH = %.c %.S $(SRCDIR)

# Generic code
CORE_OBJS = init.o debug.o

all: $(TARGET)


%.o: %.S
	$(CC) -c $(CFLAGS) -D__ASSEMBLY__ -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

#
# Make targets
#
.PHONY: all build clean

all: $(TARGET).bin


$(TARGET).bin: $(TARGET)
	$(OBJCOPY) -R -S -O binary -R .note -R .note.gnu.build-id -R .comment $(OUTPUTDIR)/$< $(OUTPUTDIR)/$@

$(TARGET): $(BOOT_LAYOUT) $(CORE_OBJS)
	mkdir -p $(OUTPUTDIR)
	$(LD) -o $(OUTPUTDIR)/$@ $(CORE_OBJS) $(LDFLAGS)

sb: $(TARGET).bin
	./elftosb2 -V -c $(SRCDIR)/$(TARGET).db -o $(OUTPUTDIR)/$(TARGET)_sb.bin
	dd if=/dev/zero of=$(OUTPUTDIR)/$(TARGET).raw bs=512 count=4
	dd if=$(OUTPUTDIR)/$(TARGET)_sb.bin of=$(OUTPUTDIR)/$(TARGET).raw ibs=512 seek=4 conv=sync,notrunc
	sudo dd if=$(OUTPUTDIR)/$(TARGET).raw of=/dev/mmcblk0p1



clean:
	@echo Cleaning...
	@echo Files:
	@echo Build output:
	rm -rf *.o
	rm -rf $(TARGET) $(OUTPUTDIR)

