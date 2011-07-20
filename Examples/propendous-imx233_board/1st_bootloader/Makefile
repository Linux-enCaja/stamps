TARGET = 1st_bootloader

CURRENTDIR	= .
SRCDIR		= $(CURRENTDIR)/src
OUTPUTDIR	= $(CURRENTDIR)/output

INCLUDEDIR	= $(CURRENTDIR)/include

# Linker script 
BASE_ADDR	?= 0
BOOT_LAYOUT_IN	= $(SRCDIR)/link_script.ld.in
BOOT_LAYOUT_OUT	= $(OUTPUTDIR)/link_script.ld


# Output ELF image
$(TARGET)_ELF	= $(OUTPUTDIR)/$(TARGET).elf

# Output binary image
$(TARGET)_BIN	= $(OUTPUTDIR)/$(TARGET).bin

CROSS_COMPILE ?= arm-elf-eabi-

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
SRC_OBJS = entry.o main.o hardware_init.o sd.o


$(TARGET)_OBJS = $(addprefix $(SRCDIR)/, $(SRC_OBJS))

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

build: build_prep $($(TARGET)_BIN)

build_prep:
	mkdir -p $(OUTPUTDIR)

clean:
	@echo Cleaning...
	@echo Files:
	rm -rf $($(TARGET)_OBJS) $(BOOT_LAYOUT_OUT)
	@echo Build output:
	rm -rf $(OUTPUTDIR)

##
## Rules to build linux_prep image
## 
#$(CMDLINES_STRIP): $(CMDLINES)
#	$(call strip_cmdlines_file)

$($(TARGET)_BIN): $($(TARGET)_ELF)
	$(OBJCOPY) -R -S -O binary -R .note -R .note.gnu.build-id -R .comment $< $@

$($(TARGET)_ELF): $($(TARGET)_OBJS) $(BOOT_LAYOUT_OUT)
	$(LD) -o $@ $($(TARGET)_OBJS) $(LDFLAGS)
	@nm -n $@ > $@.map

$(BOOT_LAYOUT_OUT): $(BOOT_LAYOUT_IN)
	$(CPP) -P -DBASE_ADDR=$(BASE_ADDR) -o $@ $<

