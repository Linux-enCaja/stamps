#include "regspinctrl.h"
#include "regsdigctl.h"

/* LED pin lives in PWM, bank 1, pin 28 */
#define LED (1<<6)


void delay_us (int us)
{
   volatile int start = HW_DIGCTL_MICROSECONDS_RD();

   while (HW_DIGCTL_MICROSECONDS_RD() - start < us);
}

int main(void)
{

/*
regutil -w HW_PINCTRL_MUXSEL0_SET=0x00000003     PIN0  -> GPIO
regutil -w HW_PINCTRL_DOE0_SET=0x00000001        PIN0  -> output
regutil -w HW_PINCTRL_DRIVE0_SET=0x3            PIN0  -> 12 mA
regutil -w HW_PINCTRL_DOUT0_SET=0x00000001        PIN0  -> high
regutil -w HW_PINCTRL_DOUT0_CLR=0x00000001        PIN0  -> low
regutil -w HW_PINCTRL_DOUT0_TOG=0x00000001        PIN0  -> toggle
*/


    /* Enable parallel JTAG, 12mA drive and the MUX pins for it */
    HW_PINCTRL_MUXSEL0_CLR(0x3FFF);
    HW_PINCTRL_MUXSEL0_SET(0x3000);


    HW_PINCTRL_DRIVE0_CLR(0x3FFF);
    HW_PINCTRL_DRIVE0_SET(0x2000000);



    /* Configue PIO for LED */
    HW_PINCTRL_DOUT0_SET(LED);
    HW_PINCTRL_DOE0_SET(LED);

//    HW_PINCTRL_DOUT0_SET(nMOS);
//    HW_PINCTRL_DOE0_SET(nMOS);
    
    while(1)
    {
        HW_PINCTRL_DOUT0_SET(LED);
        delay_us(500000);
        HW_PINCTRL_DOUT0_CLR(LED);
        delay_us(500000);
    }

    return 0;
}
