#define STR serial_puts

#define HW_CLKCTRL_CPU_WR(x) ((*((int *)0x80040020))=x)
#define HW_CLKCTRL_CPU_RD() ((*((int *)0x80040020)))
#define HW_CLKCTRL_HBUS_WR(x) ((*((int *)0x80040030))=x)
#define HW_CLKCTRL_HBUS_RD() ((*((int *)0x80040030)))
#define HW_OCOTP_CTRL_WR(x) ((*((int *)0x8002c000))=x)
#define HW_OCOTP_CTRL_SET(x) ((*((int *)0x8002c004))=x)
#define HW_OCOTP_CTRL_CLR(x) ((*((int *)0x8002c008))=x)
#define HW_OCOTP_CTRL_TOG(x) ((*((int *)0x8002c00c))=x)
#define HW_OCOTP_DATA_WR(x) ((*((int *)0x8002c010))=x)
#define HW_OCOTP_CUST0_WR(x) ((*((int *)0x8002c020))=x)
#define HW_OCOTP_CUST1_WR(x) ((*((int *)0x8002c030))=x)
#define HW_OCOTP_CUST2_WR(x) ((*((int *)0x8002c040))=x)
#define HW_OCOTP_CUST3_WR(x) ((*((int *)0x8002c050))=x)
#define HW_OCOTP_CRYPTO0_WR(x) ((*((int *)0x8002c060))=x)
#define HW_OCOTP_CRYPTO1_WR(x) ((*((int *)0x8002c070))=x)
#define HW_OCOTP_CRYPTO2_WR(x) ((*((int *)0x8002c080))=x)
#define HW_OCOTP_CRYPTO3_WR(x) ((*((int *)0x8002c090))=x)
#define HW_OCOTP_HWCAP0_WR(x) ((*((int *)0x8002c0a0))=x)
#define HW_OCOTP_HWCAP1_WR(x) ((*((int *)0x8002c0b0))=x)
#define HW_OCOTP_HWCAP2_WR(x) ((*((int *)0x8002c0c0))=x)
#define HW_OCOTP_HWCAP3_WR(x) ((*((int *)0x8002c0d0))=x)
#define HW_OCOTP_HWCAP4_WR(x) ((*((int *)0x8002c0e0))=x)
#define HW_OCOTP_HWCAP5_WR(x) ((*((int *)0x8002c0f0))=x)
#define HW_OCOTP_SWCAP_WR(x) ((*((int *)0x8002c100))=x)
#define HW_OCOTP_CUSTCAP_WR(x) ((*((int *)0x8002c110))=x)
#define HW_OCOTP_LOCK_WR(x) ((*((int *)0x8002c120))=x)
#define HW_OCOTP_OPS0_WR(x) ((*((int *)0x8002c130))=x)
#define HW_OCOTP_OPS1_WR(x) ((*((int *)0x8002c140))=x)
#define HW_OCOTP_OPS2_WR(x) ((*((int *)0x8002c150))=x)
#define HW_OCOTP_OPS3_WR(x) ((*((int *)0x8002c160))=x)
#define HW_OCOTP_UN0_WR(x) ((*((int *)0x8002c170))=x)
#define HW_OCOTP_UN1_WR(x) ((*((int *)0x8002c180))=x)
#define HW_OCOTP_UN2_WR(x) ((*((int *)0x8002c190))=x)
#define HW_OCOTP_ROM0_WR(x) ((*((int *)0x8002c1a0))=x)
#define HW_OCOTP_ROM1_WR(x) ((*((int *)0x8002c1b0))=x)
#define HW_OCOTP_ROM2_WR(x) ((*((int *)0x8002c1c0))=x)
#define HW_OCOTP_ROM3_WR(x) ((*((int *)0x8002c1d0))=x)
#define HW_OCOTP_ROM4_WR(x) ((*((int *)0x8002c1e0))=x)
#define HW_OCOTP_ROM5_WR(x) ((*((int *)0x8002c1f0))=x)
#define HW_OCOTP_ROM6_WR(x) ((*((int *)0x8002c200))=x)
#define HW_OCOTP_ROM7_WR(x) ((*((int *)0x8002c210))=x)
#define HW_OCOTP_VERSION_WR(x) ((*((int *)0x8002c220))=x)
#define HW_OCOTP_CTRL_RD() ((*((int *)0x8002c000)))
#define HW_OCOTP_DATA_RD() ((*((int *)0x8002c010)))
#define HW_OCOTP_CUST0_RD() ((*((int *)0x8002c020)))
#define HW_OCOTP_CUST1_RD() ((*((int *)0x8002c030)))
#define HW_OCOTP_CUST2_RD() ((*((int *)0x8002c040)))
#define HW_OCOTP_CUST3_RD() ((*((int *)0x8002c050)))
#define HW_OCOTP_CRYPTO0_RD() ((*((int *)0x8002c060)))
#define HW_OCOTP_CRYPTO1_RD() ((*((int *)0x8002c070)))
#define HW_OCOTP_CRYPTO2_RD() ((*((int *)0x8002c080)))
#define HW_OCOTP_CRYPTO3_RD() ((*((int *)0x8002c090)))
#define HW_OCOTP_HWCAP0_RD() ((*((int *)0x8002c0a0)))
#define HW_OCOTP_HWCAP1_RD() ((*((int *)0x8002c0b0)))
#define HW_OCOTP_HWCAP2_RD() ((*((int *)0x8002c0c0)))
#define HW_OCOTP_HWCAP3_RD() ((*((int *)0x8002c0d0)))
#define HW_OCOTP_HWCAP4_RD() ((*((int *)0x8002c0e0)))
#define HW_OCOTP_HWCAP5_RD() ((*((int *)0x8002c0f0)))
#define HW_OCOTP_SWCAP_RD() ((*((int *)0x8002c100)))
#define HW_OCOTP_CUSTCAP_RD() ((*((int *)0x8002c110)))
#define HW_OCOTP_LOCK_RD() ((*((int *)0x8002c120)))
#define HW_OCOTP_OPS0_RD() ((*((int *)0x8002c130)))
#define HW_OCOTP_OPS1_RD() ((*((int *)0x8002c140)))
#define HW_OCOTP_OPS2_RD() ((*((int *)0x8002c150)))
#define HW_OCOTP_OPS3_RD() ((*((int *)0x8002c160)))
#define HW_OCOTP_UN0_RD() ((*((int *)0x8002c170)))
#define HW_OCOTP_UN1_RD() ((*((int *)0x8002c180)))
#define HW_OCOTP_UN2_RD() ((*((int *)0x8002c190)))
#define HW_OCOTP_ROM0_RD() ((*((int *)0x8002c1a0)))
#define HW_OCOTP_ROM1_RD() ((*((int *)0x8002c1b0)))
#define HW_OCOTP_ROM2_RD() ((*((int *)0x8002c1c0)))
#define HW_OCOTP_ROM3_RD() ((*((int *)0x8002c1d0)))
#define HW_OCOTP_ROM4_RD() ((*((int *)0x8002c1e0)))
#define HW_OCOTP_ROM5_RD() ((*((int *)0x8002c1f0)))
#define HW_OCOTP_ROM6_RD() ((*((int *)0x8002c200)))
#define HW_OCOTP_ROM7_RD() ((*((int *)0x8002c210)))
#define HW_OCOTP_VERSION_RD() ((*((int *)0x8002c220)))
#define HW_POWER_VDDIOCTRL_WR(x) ((*((int *) 0x80044060))=x)
#define HW_POWER_VDDIOCTRL_RD() ((*((int *) 0x80044060)))
#define HW_DIGCTL_MICROSECONDS_RD() ((*((int *)0x8001C0C0)))


#define STR shell_write_str
#define INT shell_write_int
#define HEX shell_write_hex
#define CHR shell_write_chr
#define shell_init serial_init


int otp_write(int address, int data) {
    int previous_vddio, start_us;
    /*
    STR("Not programming OTP!  Wanted to write "), HEX(address), 
        STR(" as "), HEX(data), STR("\n");
    return 1;
    */

    // Set the H_CLK to 24 MHz.  The PLL is running at 480 MHz, and
    // the CPU should be running at 480 MHz as well.
    if(HW_CLKCTRL_CPU_RD()!=0x00010001)
        return 1;
    HW_CLKCTRL_HBUS_WR(0x00000014);

    // Make sure VDDIO is running at 2.8V
    previous_vddio = HW_POWER_VDDIOCTRL_RD();
    HW_POWER_VDDIOCTRL_WR(previous_vddio&~0x31);



    // Make sure registers aren't open for writing.
    HW_OCOTP_CTRL_CLR(1<<12);
    while(HW_OCOTP_CTRL_RD()&(1<<8));
    if(HW_OCOTP_CTRL_RD()&(1<<12)) {
        // Bring it back to the previous VDDIO.
        HW_POWER_VDDIOCTRL_WR(previous_vddio);
        return 2;
    }



    // Make sure there aren't any errors starting out.
    HW_OCOTP_CTRL_CLR(1<<9);
    if(HW_OCOTP_CTRL_RD()&(1<<9)) {
        // Bring it back to the previous VDDIO.
        HW_POWER_VDDIOCTRL_WR(previous_vddio);
        return 3;
    }


    // Write the "Unlock" code and the target register value.
    HW_OCOTP_CTRL_WR(0x3E77<<16 | address);


    // Now write the data to the data register.  This triggers programming.
    HW_OCOTP_DATA_WR(data);


    while(HW_OCOTP_CTRL_RD()&(1<<8));

    // Wait 2us after burning to let it settle, as per Freescale doc.
    start_us = HW_DIGCTL_MICROSECONDS_RD();
    while(HW_DIGCTL_MICROSECONDS_RD() < start_us+2);


    // Bring it back to the previous VDDIO.
    HW_POWER_VDDIOCTRL_WR(previous_vddio);


    // Re-load the shadow registers.
    HW_OCOTP_CTRL_WR(1<<12);
    while(HW_OCOTP_CTRL_RD()&(1<<8));


    return 0;
}


int otp_open_read() {
    HW_OCOTP_CTRL_SET(1<<12);
    while(HW_OCOTP_CTRL_RD()&(1<<8));
    return 0;
}

int otp_close_read() {
    HW_OCOTP_CTRL_CLR(1<<12);
    while(HW_OCOTP_CTRL_RD()&(1<<8));
}

