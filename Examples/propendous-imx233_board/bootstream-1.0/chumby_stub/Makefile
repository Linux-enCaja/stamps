CURRENTDIR	= .
SRCDIR		= $(CURRENTDIR)/src
OUTPUTDIR	= $(CURRENTDIR)/output

INCLUDEDIR	= $(CURRENTDIR)/include

# Linker script 
BASE_ADDR	?= 0x00002000
BOOT_LAYOUT_IN	= $(SRCDIR)/chumby_stub.ld.in
BOOT_LAYOUT_OUT	= $(OUTPUTDIR)/chumby_stub.ld


# Output ELF image
CHUMBY_STUB	= $(OUTPUTDIR)/../../images/chumby_stub

# Output binary image
CHUMBY_STUB_ROM	= $(OUTPUTDIR)/../../images/chumby_stub.rom

CROSS_COMPILE ?= arm-linux-

AS	= $(CROSS_COMPILE)as
CC	= $(CROSS_COMPILE)gcc
LD	= $(CROSS_COMPILE)ld
CPP	= $(CROSS_COMPILE)cpp
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP	= $(CROSS_COMPILE)objdump

LIBGCCDIR = $(dir $(shell $(CC) -print-libgcc-file-name))
CFLAGS 	= -Wall -I$(INCLUDEDIR) -I$(LINUX_DIR)/include -nostdinc -fno-builtin -Os
LDFLAGS = -static -nostdlib -T $(BOOT_LAYOUT_OUT) -L$(LIBGCCDIR) -lgcc

CFLAGS += -DCHUMBY_USB_FIX -DCHUMBY_SHELL


# Generic code
SRC_OBJS = entry.o  serial.o  shell.o  utils.o  sd.o


CHUMBY_STUB_OBJS = $(addprefix $(SRCDIR)/, $(SRC_OBJS))
#		  $(addprefix $(BOARDDIR)/, $(BOARD_OBJS)) \
#		  $(addprefix $(HWDIR)/, $(HW_OBJS))

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

build: build_prep $(CHUMBY_STUB_ROM)

build_prep:
	mkdir -p $(OUTPUTDIR)

clean:
	@echo Cleaning...
	@echo Files:
	rm -rf $(CHUMBY_STUB_OBJS) $(BOOT_LAYOUT_OUT)
	@echo Build output:
	rm -rf $(OUTPUTDIR)
	@echo Build system:
	rm -rf $(INCLUDEDIR)/arch

##
## Rules to build linux_prep image
## 
#$(CMDLINES_STRIP): $(CMDLINES)
#	$(call strip_cmdlines_file)

$(CHUMBY_STUB_ROM): $(CHUMBY_STUB)
	$(OBJCOPY) -R -S -O binary -R .note -R .note.gnu.build-id -R .comment $< $@

$(CHUMBY_STUB): $(CHUMBY_STUB_OBJS) $(BOOT_LAYOUT_OUT)
	$(LD) -o $@ $(CHUMBY_STUB_OBJS) $(LDFLAGS)
	@nm -n $@ > $@.map

$(BOOT_LAYOUT_OUT): $(BOOT_LAYOUT_IN)
	$(CPP) -P -DBASE_ADDR=$(BASE_ADDR) -o $@ $<

