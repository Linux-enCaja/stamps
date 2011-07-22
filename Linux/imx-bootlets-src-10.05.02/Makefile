export CROSS_COMPILE=arm-none-linux-gnueabi-
MEM_TYPE ?= MEM_DDR1
export MEM_TYPE

BOARD ?= stmp378x_dev
ARCH = mx23

all: build_prep gen_bootstream

build_prep:


gen_bootstream: linux_prep boot_prep power_prep
	@echo "generating linux kernel boot stream image"
	@echo "by using the pre-built kernel"
	elftosb2 -c linux_prebuilt.db -o i$(ARCH)_linux.sb
#	elftosb2 -c linux.db -o i$(ARCH)_linux.sb
	@echo "generating U-Boot boot stream image"
	elftosb2 -c ./uboot_prebuilt.db -o i$(ARCH)_uboot.sb

	rm -f sd_mmc_bootstream.raw
	dd if=/dev/zero of=sd_mmc_bootstream.raw bs=512 count=4
	dd if=imx23_linux.sb of=sd_mmc_bootstream.raw ibs=512 seek=4 conv=sync,notrunc
	sudo dd if=sd_mmc_bootstream.raw of=/dev/mmcblk0p1
	sudo umount /dev/mmcblk0p*

power_prep:
	@echo "build power_prep"
	$(MAKE) -C power_prep ARCH=$(ARCH) BOARD=$(BOARD)

boot_prep:
	@echo "build boot_prep"
	$(MAKE) -C boot_prep  ARCH=$(ARCH) BOARD=$(BOARD)

linux_prep:
	@echo "by using the pre-build command line"
	$(MAKE) clean -C linux_prep
	@echo "cross-compiling linux_prep"
	$(MAKE) -C linux_prep ARCH=$(ARCH) BOARD=$(BOARD)

distclean: clean
clean:
	-rm -rf *.sb
	rm -f sd_mmc_bootstream.raw
	$(MAKE) -C linux_prep clean ARCH=$(ARCH)
	$(MAKE) -C boot_prep clean ARCH=$(ARCH)
	$(MAKE) -C power_prep clean ARCH=$(ARCH)

.PHONY: all build_prep linux_prep boot_prep power_prep distclean clean

