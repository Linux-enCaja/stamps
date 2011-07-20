# $Id: Makefile 7554 2009-04-30 19:44:17Z scross $
# source build makefile for bootstream bootloader

include ../config/config.mk

ELFTOSB2_DIR=elftosb2
ELFTOSB2_BINARIES=elftosb2
ELFTOSB2_SRC_BINARIES=$(addprefix $(ELFTOSB2_DIR)/,$(ELFTOSB2_BINARIES))
ELFTOSB2_SOURCES=$(wildcard $(ELFTOSB2_DIR)/*.c $(ELFTOSB2_DIR)/*.h)

SRC_BINARIES=$(ELFTOSB2_SRC_BINARIES)
HOST_BINARIES=$(ELFTOSB2_BINARIES)

CFLAGS += -DCHUMBY_CONFIGNAME=\"$(CONFIGNAME)\" -DCHUMBY_CONFIGNAME_$(CONFIGNAME)

all: $(SRC_BINARIES) $(HOST_BINARIES) \
	chumby_factory/chumby_factory chumby_stub/chumby_stub chumby_boot/chumby_boot


install: $(SRC_BINARIES) $(HOST_BINARIES) \
	.src_binaries .chumby-boot .factory-image .chumby-stub
    #.u-boot-image

chumby_boot/chumby_boot:
	$(MAKE) -C chumby_boot CROSS_COMPILE=$(TARGET)- LINUX_DIR=$(LINUX_DIR)

chumby_stub/chumby_stub:
	$(MAKE) -C chumby_stub CROSS_COMPILE=$(TARGET)- LINUX_DIR=$(LINUX_DIR)

chumby_factory/chumby_factory:
	$(MAKE) -C chumby_factory CROSS_COMPILE=$(TARGET)- LINUX_DIR=$(LINUX_DIR)

linux_prep/linux_prep:
	$(MAKE) -C linux_prep CROSS_COMPILE=$(TARGET)- LINUX_DIR=$(LINUX_DIR)


.factory-image: $(SRC_BINARIES)
	elftosb2/elftosb2 -V -z -c ../config/falconwing_factory_sb.db \
            -o $(INSTALL_DIR)/bootstream-factory.bin

.chumby-boot: $(SRC_BINARIES)
	cp images/chumby_boot.rom $(INSTALL_DIR)/chumby_boot.bin

.chumby-stub: $(SRC_BINARIES)
	elftosb2/elftosb2 -c ../config/falconwing_chumby_sb.db \
            -o $(INSTALL_DIR)/bootstream-chumby.bin

.u-boot-image: $(SRC_BINARIES)
	elftosb2/elftosb2 -c ../config/falconwing_uboot_sb.db \
            -o $(INSTALL_DIR)/falconwing_uboot.sb


.src_binaries:
	@echo "Copying src bianries"
	mkdir -p $(INSTALL_DIR)/host
	cp --preserve --no-dereference $(SRC_BINARIES) $(INSTALL_DIR)/host/

clean:
	-rm -f $(HOST_BINARIES) $(SRC_BINARIES)
	$(MAKE) -C $(ELFTOSB2_DIR) clean

$(ELFTOSB2_SRC_BINARIES):
	$(MAKE) -C $(ELFTOSB2_DIR)

../output/$(TARGET)-$(CNPLATFORM):
	@echo "Creating directory: $@"
	-mkdir -p $@
