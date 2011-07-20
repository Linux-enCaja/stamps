/* ========================================================================== */
/*                                                                            */
/*   hardware_init.c                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/*   Hardware setup                                                           */
/* ========================================================================== */

#include "regspinctrl.h"
#include "regsdigctl.h"
#include "regsclkctrl.h"
#include "regspower.h"
#include "regsemi.h"
#include "regsdram.h"

#include "hardware_init.h"

#define PIN_DRIVE_12mA      2

void delay_us (int us)
{
   volatile int start = HW_DIGCTL_MICROSECONDS_RD();

   while (HW_DIGCTL_MICROSECONDS_RD() - start < us)
       ;
}

static void poweron_pll()
{
    HW_CLKCTRL_PLLCTRL0_SET(BM_CLKCTRL_PLLCTRL0_POWER);
}

/* Enables VDDMEM, for external RAM */
static void turnon_mem_rail(int mv)
{
    unsigned int value;
    HW_POWER_CTRL_CLR(BM_POWER_CTRL_CLKGATE);

    value = BM_POWER_VDDMEMCTRL_ENABLE_ILIMIT|
        BM_POWER_VDDMEMCTRL_ENABLE_LINREG|
        BM_POWER_VDDMEMCTRL_PULLDOWN_ACTIVE|
        (mv-1700)/50;

    HW_POWER_VDDMEMCTRL_WR(value);
    delay_us(20000);
    value &= ~(BM_POWER_VDDMEMCTRL_ENABLE_ILIMIT|
         BM_POWER_VDDMEMCTRL_PULLDOWN_ACTIVE);
    HW_POWER_VDDMEMCTRL_WR(value);
}

#define PIN_VOL(pin , v) ((v) ? (pin) : 0)

static void init_emi_pin(int pin_voltage,
          int pin_drive
          )
{
    /* Reset and enable PINCTRL block */
    HW_PINCTRL_CTRL_CLR(BM_PINCTRL_CTRL_SFTRST | BM_PINCTRL_CTRL_CLKGATE);

    /* EMI_A00-06 */
    /* Configure Bank-2 Pins 9-15 voltage and drive strength*/
    HW_PINCTRL_DRIVE9_CLR(
        BM_PINCTRL_DRIVE9_BANK2_PIN09_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN09_MA |
        BM_PINCTRL_DRIVE9_BANK2_PIN10_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN10_MA |
        BM_PINCTRL_DRIVE9_BANK2_PIN11_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN11_MA |
        BM_PINCTRL_DRIVE9_BANK2_PIN12_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN12_MA |
        BM_PINCTRL_DRIVE9_BANK2_PIN13_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN13_MA |
        BM_PINCTRL_DRIVE9_BANK2_PIN14_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN14_MA |
        BM_PINCTRL_DRIVE9_BANK2_PIN15_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN15_MA);

    HW_PINCTRL_DRIVE9_SET(
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN09_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN09_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN10_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN10_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN11_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN11_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN12_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN12_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN13_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN13_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN14_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN14_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN15_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN15_MA(pin_drive));

    /* EMI_A07-12, EMI_BA0-1 */
    /* Configure Bank-2 Pins 16-23 voltage and drive strength */
    HW_PINCTRL_DRIVE10_CLR(
        BM_PINCTRL_DRIVE10_BANK2_PIN16_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN16_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN17_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN17_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN18_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN18_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN19_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN19_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN20_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN20_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN21_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN21_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN22_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN22_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN23_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN23_MA);

    HW_PINCTRL_DRIVE10_SET(
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN16_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN16_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN17_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN17_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN18_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN18_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN19_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN19_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN20_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN20_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN21_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN21_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN22_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN22_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN23_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN23_MA(pin_drive));

    /* EMI_CAS,RAS,CE0-2,WEN,CKE */
    /* Configure Bank-2 Pins 24-31 voltage and drive strength */
    HW_PINCTRL_DRIVE11_CLR(
        BM_PINCTRL_DRIVE11_BANK2_PIN24_V |
        BM_PINCTRL_DRIVE11_BANK2_PIN24_MA |
        BM_PINCTRL_DRIVE11_BANK2_PIN25_V |
        BM_PINCTRL_DRIVE11_BANK2_PIN25_MA |
        BM_PINCTRL_DRIVE11_BANK2_PIN26_V |
        BM_PINCTRL_DRIVE11_BANK2_PIN26_MA |
        BM_PINCTRL_DRIVE11_BANK2_PIN29_V |
        BM_PINCTRL_DRIVE11_BANK2_PIN29_MA |
        BM_PINCTRL_DRIVE11_BANK2_PIN30_V |
        BM_PINCTRL_DRIVE11_BANK2_PIN30_MA |
        BM_PINCTRL_DRIVE11_BANK2_PIN31_V |
        BM_PINCTRL_DRIVE11_BANK2_PIN31_MA);

    HW_PINCTRL_DRIVE11_SET(
        PIN_VOL(BM_PINCTRL_DRIVE11_BANK2_PIN24_V , pin_voltage) |
        BF_PINCTRL_DRIVE11_BANK2_PIN24_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE11_BANK2_PIN25_V , pin_voltage) |
        BF_PINCTRL_DRIVE11_BANK2_PIN25_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE11_BANK2_PIN26_V , pin_voltage) |
        BF_PINCTRL_DRIVE11_BANK2_PIN26_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE11_BANK2_PIN29_V , pin_voltage) |
        BF_PINCTRL_DRIVE11_BANK2_PIN29_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE11_BANK2_PIN30_V , pin_voltage) |
        BF_PINCTRL_DRIVE11_BANK2_PIN30_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE11_BANK2_PIN31_V , pin_voltage) |
        BF_PINCTRL_DRIVE11_BANK2_PIN31_MA(pin_drive));

    /* Configure Bank-2 Pins 9-15 as EMI pins */
    HW_PINCTRL_MUXSEL4_CLR(
        BM_PINCTRL_MUXSEL4_BANK2_PIN09 |
        BM_PINCTRL_MUXSEL4_BANK2_PIN10 |
        BM_PINCTRL_MUXSEL4_BANK2_PIN11 |
        BM_PINCTRL_MUXSEL4_BANK2_PIN12 |
        BM_PINCTRL_MUXSEL4_BANK2_PIN13 |
        BM_PINCTRL_MUXSEL4_BANK2_PIN14 |
        BM_PINCTRL_MUXSEL4_BANK2_PIN15);

    /* Configure Bank-2 Pins 16-31 as EMI pins */
    HW_PINCTRL_MUXSEL5_CLR(
        BM_PINCTRL_MUXSEL5_BANK2_PIN16 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN17 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN18 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN19 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN20 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN21 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN22 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN23 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN24 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN25 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN26 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN29 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN30 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN31);

    HW_PINCTRL_DRIVE12_CLR(
        BM_PINCTRL_DRIVE12_BANK3_PIN00_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN00_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN01_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN01_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN02_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN02_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN03_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN03_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN04_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN04_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN05_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN05_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN06_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN06_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN07_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN07_MA);

    HW_PINCTRL_DRIVE12_SET(
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN00_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN00_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN01_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN01_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN02_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN02_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN03_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN03_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN04_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN04_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN05_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN05_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN06_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN06_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN07_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN07_MA(pin_drive));

    /* EMI_D08-15
      Configure Bank-3 Pins 08-15 voltage and drive strength
    */
    HW_PINCTRL_DRIVE13_CLR(
        BM_PINCTRL_DRIVE13_BANK3_PIN08_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN08_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN09_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN09_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN10_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN10_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN11_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN11_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN12_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN12_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN13_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN13_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN14_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN14_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN15_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN15_MA);

    HW_PINCTRL_DRIVE13_SET(
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN08_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN08_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN09_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN09_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN10_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN10_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN11_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN11_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN12_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN12_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN13_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN13_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN14_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN14_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN15_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN15_MA(pin_drive));

    /* EMI_DQS0-1,DQM0-1,CLK,CLKN
       Configure Bank-3 Pins 08-15 voltage and drive strength
     */
    HW_PINCTRL_DRIVE14_CLR(
        BM_PINCTRL_DRIVE14_BANK3_PIN16_V  |
        BM_PINCTRL_DRIVE14_BANK3_PIN16_MA |
        BM_PINCTRL_DRIVE14_BANK3_PIN17_V  |
        BM_PINCTRL_DRIVE14_BANK3_PIN17_MA |
        BM_PINCTRL_DRIVE14_BANK3_PIN18_V  |
        BM_PINCTRL_DRIVE14_BANK3_PIN18_MA |
        BM_PINCTRL_DRIVE14_BANK3_PIN19_V  |
        BM_PINCTRL_DRIVE14_BANK3_PIN19_MA |
        BM_PINCTRL_DRIVE14_BANK3_PIN20_V  |
        BM_PINCTRL_DRIVE14_BANK3_PIN20_MA |
        BM_PINCTRL_DRIVE14_BANK3_PIN21_V  |
        BM_PINCTRL_DRIVE14_BANK3_PIN21_MA);

    HW_PINCTRL_DRIVE14_SET(
        PIN_VOL(BM_PINCTRL_DRIVE14_BANK3_PIN16_V , pin_voltage) |
        BF_PINCTRL_DRIVE14_BANK3_PIN16_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE14_BANK3_PIN17_V , pin_voltage) |
        BF_PINCTRL_DRIVE14_BANK3_PIN17_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE14_BANK3_PIN18_V , pin_voltage) |
        BF_PINCTRL_DRIVE14_BANK3_PIN18_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE14_BANK3_PIN19_V , pin_voltage) |
        BF_PINCTRL_DRIVE14_BANK3_PIN19_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE14_BANK3_PIN20_V , pin_voltage) |
        BF_PINCTRL_DRIVE14_BANK3_PIN20_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE14_BANK3_PIN21_V , pin_voltage) |
        BF_PINCTRL_DRIVE14_BANK3_PIN21_MA(pin_drive));

    /* Configure Bank-3 Pins 0-15 as EMI pins*/
    HW_PINCTRL_MUXSEL6_CLR(
        BM_PINCTRL_MUXSEL6_BANK3_PIN00 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN01 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN02 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN03 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN04 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN05 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN06 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN07 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN08 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN09 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN10 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN11 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN12 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN13 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN14 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN15);

    /* Configure Bank-3 Pins 16-21 as EMI pins */
    HW_PINCTRL_MUXSEL7_CLR(
        BM_PINCTRL_MUXSEL7_BANK3_PIN16 |
        BM_PINCTRL_MUXSEL7_BANK3_PIN17 |
        BM_PINCTRL_MUXSEL7_BANK3_PIN18 |
        BM_PINCTRL_MUXSEL7_BANK3_PIN19 |
        BM_PINCTRL_MUXSEL7_BANK3_PIN20 |
        BM_PINCTRL_MUXSEL7_BANK3_PIN21);
}

static void disable_emi_padkeepers(void)
{
    HW_PINCTRL_CTRL_CLR(BM_PINCTRL_CTRL_SFTRST | BM_PINCTRL_CTRL_CLKGATE);

    HW_PINCTRL_PULL3_SET(
    BM_PINCTRL_PULL3_BANK3_PIN17 |
    BM_PINCTRL_PULL3_BANK3_PIN16 |
    BM_PINCTRL_PULL3_BANK3_PIN15 |
    BM_PINCTRL_PULL3_BANK3_PIN14 |
    BM_PINCTRL_PULL3_BANK3_PIN13 |
    BM_PINCTRL_PULL3_BANK3_PIN12 |
    BM_PINCTRL_PULL3_BANK3_PIN11 |
    BM_PINCTRL_PULL3_BANK3_PIN10 |
    BM_PINCTRL_PULL3_BANK3_PIN09 |
    BM_PINCTRL_PULL3_BANK3_PIN08 |
    BM_PINCTRL_PULL3_BANK3_PIN07 |
    BM_PINCTRL_PULL3_BANK3_PIN06 |
    BM_PINCTRL_PULL3_BANK3_PIN05 |
    BM_PINCTRL_PULL3_BANK3_PIN04 |
    BM_PINCTRL_PULL3_BANK3_PIN03 |
    BM_PINCTRL_PULL3_BANK3_PIN02 |
    BM_PINCTRL_PULL3_BANK3_PIN01 |
    BM_PINCTRL_PULL3_BANK3_PIN00);

}

static void set_emi_frac(unsigned int div)
{
    HW_CLKCTRL_FRAC_SET(BM_CLKCTRL_FRAC_EMIFRAC);
    div = (~div);
    HW_CLKCTRL_FRAC_CLR(BF_CLKCTRL_FRAC_EMIFRAC(div));
}

static void init_ddr_W9425G6EH_5_96Mhz(int ce)
{
    HW_DRAM_CTL00_WR(0x01010001);
    HW_DRAM_CTL01_WR(0x00010000);
    HW_DRAM_CTL02_WR(0x01000000);
    HW_DRAM_CTL03_WR(0x00000001);
    HW_DRAM_CTL04_WR(0x00000101);
    HW_DRAM_CTL05_WR(0x00000000);
    HW_DRAM_CTL06_WR(0x00010000);
    HW_DRAM_CTL07_WR(0x01000001);
    HW_DRAM_CTL09_WR(0x00000001);
    HW_DRAM_CTL10_WR(0x07000200);
    HW_DRAM_CTL11_WR(0x00070302);
    HW_DRAM_CTL12_WR(0x02020000);
    HW_DRAM_CTL13_WR(0x04040a01);
    HW_DRAM_CTL14_WR(0x00000200|ce);
    HW_DRAM_CTL15_WR(0x02040000);
    HW_DRAM_CTL16_WR(0x02000000);
    HW_DRAM_CTL17_WR(0x25001506);
    HW_DRAM_CTL18_WR(0x1f1f0000);
    HW_DRAM_CTL19_WR(0x027f1a1a);
    HW_DRAM_CTL20_WR(0x02051c22);
    HW_DRAM_CTL21_WR(0x00000007);
    HW_DRAM_CTL22_WR(0x00080008);
    HW_DRAM_CTL23_WR(0x00200020);
    HW_DRAM_CTL24_WR(0x00200020);
    HW_DRAM_CTL25_WR(0x00200020);
    HW_DRAM_CTL26_WR(0x000002e6);
    HW_DRAM_CTL29_WR(0x00000020);
    HW_DRAM_CTL30_WR(0x00000020);
    HW_DRAM_CTL31_WR(0x00c80000);
    HW_DRAM_CTL32_WR(0x00081a3b);
    HW_DRAM_CTL33_WR(0x000000c8);
    HW_DRAM_CTL34_WR(0x00004b0d);
    HW_DRAM_CTL36_WR(0x00000101);
    HW_DRAM_CTL37_WR(0x00040001);
    HW_DRAM_CTL38_WR(0x00000000);
    HW_DRAM_CTL39_WR(0x00000000);
    HW_DRAM_CTL40_WR(0x00010000);
    HW_DRAM_CTL08_WR(0x01000000);
}

static void init_clock()
{
    /* ref_emi clock is off */
    HW_CLKCTRL_FRAC_SET(BM_CLKCTRL_FRAC_CLKGATEEMI);

    /* init 261,8181 MHz... */
    set_emi_frac(33);

    /* ref_emi clock is on */
    HW_CLKCTRL_FRAC_CLR(BM_CLKCTRL_FRAC_CLKGATEEMI);
    delay_us(11000);

    /* now divide 288/2 = 130,9090 MHz <== ref_emi */
    HW_CLKCTRL_EMI_WR(BF_CLKCTRL_EMI_DIV_XTAL(1)|
                BF_CLKCTRL_EMI_DIV_EMI(2)
             );

    /* choose ref_emi */
    HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_EMI);

    /* reset EMI */
    HW_EMI_CTRL_CLR(BM_EMI_CTRL_SFTRST);
    HW_EMI_CTRL_CLR(BM_EMI_CTRL_CLKGATE);
}

static void exit_selfrefresh()
{
    unsigned int start;
    unsigned int value;
    value = HW_DRAM_CTL16_RD();
    value &= ~(1<<17);
    HW_DRAM_CTL16_WR(value);

    start = HW_DIGCTL_MICROSECONDS_RD();

    while ((HW_EMI_STAT_RD()&BM_EMI_STAT_DRAM_HALTED)) {

        if (HW_DIGCTL_MICROSECONDS_RD() > (start + 1000000)) {
            while (1) ;
            //return;
        }
    }
}

static void set_port_priority()
{
    HW_EMI_CTRL_CLR(BM_EMI_CTRL_PORT_PRIORITY_ORDER);
    HW_EMI_CTRL_SET(BF_EMI_CTRL_PORT_PRIORITY_ORDER(
                BV_EMI_CTRL_PORT_PRIORITY_ORDER__PORT1230)
               );

    HW_EMI_CTRL_CLR(BM_EMI_CTRL_PORT_PRIORITY_ORDER);
    HW_EMI_CTRL_SET(BF_EMI_CTRL_PORT_PRIORITY_ORDER(0x2));
}

static void entry_auto_clock_gate()
{
    unsigned int value;
    value =  HW_DRAM_CTL16_RD();
    value |= 1<<19;
    HW_DRAM_CTL16_WR(value);

    value =  HW_DRAM_CTL16_RD();
    value |= 1<<11;
    HW_DRAM_CTL16_WR(value);
}

static void change_cpu_freq()
{
    int value = 0;

    value = HW_POWER_VDDDCTRL_RD();
    value &= ~BM_POWER_VDDDCTRL_TRG;
    value |= BF_POWER_VDDDCTRL_TRG(30); /*change to 1.550v*/
    HW_POWER_VDDDCTRL_WR(value);

    delay_us(10000);

    value = HW_CLKCTRL_FRAC_RD();
    value &= ~BM_CLKCTRL_FRAC_CPUFRAC;
    value |= BF_CLKCTRL_FRAC_CPUFRAC(19);
    value &= ~BM_CLKCTRL_FRAC_CLKGATECPU;

    HW_CLKCTRL_FRAC_WR(value); /*Change cpu to 454Mhz*/

    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);

    HW_CLKCTRL_HBUS_SET(BM_CLKCTRL_HBUS_DIV);
    HW_CLKCTRL_HBUS_CLR(((~3)&BM_CLKCTRL_HBUS_DIV));

    delay_us(10000);

    HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);
}

void hardware_setup(void)
{
    unsigned int value;
    
    /* Configue PIO for LED */
    HW_PINCTRL_DOUT1_CLR(LED);
    HW_PINCTRL_DOE1_SET(LED);

    /* Enable PLL. But CPU clock should be here at 24MHz from cristal */
    poweron_pll();
    delay_us(11000);

    /* Enable the VDDMEM to 2.5V */
    turnon_mem_rail(2500);
    delay_us(11000);

    /* enable pins as for working with EMI, for external RAM */
    init_emi_pin(0, PIN_DRIVE_12mA);

    disable_emi_padkeepers();

    /* init the EMI clock at 96MHz, uses now the PLL for it */
    init_clock();
    delay_us(10000);

    /* init our 32MBytes SDRAM memory */
    init_ddr_W9425G6EH_5_96Mhz(1);

    value = HW_DRAM_CTL08_RD();
    value |= BM_DRAM_CTL08_START;
    HW_DRAM_CTL08_WR(value);

    exit_selfrefresh();

    set_port_priority();

    entry_auto_clock_gate();

    /* now CPU will run using PLL. Will run at 454MHz */
    change_cpu_freq();
}
