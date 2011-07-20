CURRENTDIR	= .
SRCDIR		= $(CURRENTDIR)/src
OUTPUTDIR	= $(CURRENTDIR)/output

INCLUDEDIR	= $(CURRENTDIR)/include

# Linker script 
BASE_ADDR	?= 0x00002000
BOOT_LAYOUT_IN	= $(SRCDIR)/chumby_boot.ld.in
BOOT_LAYOUT_OUT	= $(OUTPUTDIR)/chumby_boot.ld


# Output ELF image
CHUMBY_BOOT	= $(OUTPUTDIR)/../../images/chumby_boot

# Output binary image
CHUMBY_BOOT_ROM	= $(OUTPUTDIR)/../../images/chumby_boot.rom

CROSS_COMPILE ?= arm-linux-

AS	= $(CROSS_COMPILE)as
CC	= $(CROSS_COMPILE)gcc
LD	= $(CROSS_COMPILE)ld
CPP	= $(CROSS_COMPILE)cpp
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP	= $(CROSS_COMPILE)objdump

LIBGCCDIR = $(dir $(shell $(CC) -print-libgcc-file-name))
CFLAGS 	= -Wall -I$(INCLUDEDIR) -I$(LINUX_DIR)/include -fno-builtin -O -nostdinc
LDFLAGS = -static -nostdlib -T $(BOOT_LAYOUT_OUT) -L$(LIBGCCDIR) -lgcc

CFLAGS += -fPIC


# Generic code
SRC_OBJS = entry.o serial.o shell.o shell_funcs.o utils.o sd.o \
		   linux.o otp.o ts.o


CHUMBY_BOOT_OBJS = $(addprefix $(SRCDIR)/, $(SRC_OBJS))
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

build: build_prep $(CHUMBY_BOOT_ROM)

build_prep:
	mkdir -p $(OUTPUTDIR)

clean:
	@echo Cleaning...
	@echo Files:
	rm -rf $(CHUMBY_BOOT_OBJS) $(BOOT_LAYOUT_OUT)
	@echo Build output:
	rm -rf $(OUTPUTDIR)
	@echo Build system:
	rm -rf $(INCLUDEDIR)/arch

##
## Rules to build linux_prep image
## 
#$(CMDLINES_STRIP): $(CMDLINES)
#	$(call strip_cmdlines_file)

$(CHUMBY_BOOT_ROM): $(CHUMBY_BOOT)
	$(OBJCOPY) -R -S -O binary -R .note -R .note.gnu.build-id -R .comment $< $@

$(CHUMBY_BOOT): $(CHUMBY_BOOT_OBJS) $(BOOT_LAYOUT_OUT)
	$(LD) -o $@ $(CHUMBY_BOOT_OBJS) $(LDFLAGS)
	@nm -n $@ > $@.map

$(BOOT_LAYOUT_OUT): $(BOOT_LAYOUT_IN)
	$(CPP) -P -DBASE_ADDR=$(BASE_ADDR) -o $@ $<

