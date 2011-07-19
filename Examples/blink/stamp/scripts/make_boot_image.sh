dd if=/dev/zero of=bootstream-factory.raw bs=512 count=4
dd if=output/blink_led_sb.bin of=bootstream-factory.raw ibs=512 seek=4 conv=sync,notrunc
dd if=bootstream-factory.raw of=/dev/sdb1
./bootstream_make_bootable.pl /dev/sdb
