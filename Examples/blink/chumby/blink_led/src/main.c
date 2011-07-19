#include "regspinctrl.h"
#include "regsdigctl.h"

/* LED pin lives in PWM, bank 1, pin 28 */
#define LED (1)
#define nMOS (1<<19)

void delay_us (int us)
{
   volatile int start = HW_DIGCTL_MICROSECONDS_RD();

   while (HW_DIGCTL_MICROSECONDS_RD() - start < us)
       ;
}

int main(void)
{
    /* Enable parallel JTAG, 12mA drive and the MUX pins for it */
    HW_PINCTRL_MUXSEL0_CLR(0x3FFF);
    HW_PINCTRL_MUXSEL0_SET(0x0003);

    HW_PINCTRL_MUXSEL1_CLR(0x3FFF);
    HW_PINCTRL_MUXSEL1_SET(0x00C0);

    HW_PINCTRL_DRIVE0_CLR(0x3FFF);
    HW_PINCTRL_DRIVE0_SET(0x0020);

    HW_PINCTRL_DRIVE2_CLR(0x3FFF);
    HW_PINCTRL_DRIVE2_SET(0x2000);



    /* Enable Parallel JTAG */
    HW_DIGCTL_CTRL_CLR((1 << 1) | (1 << 3) | (1 << 6));
    /**************************************/

    /* Configue PIO for LED */
    HW_PINCTRL_DOUT0_SET(LED);
    HW_PINCTRL_DOE0_SET(LED);

    HW_PINCTRL_DOUT0_SET(nMOS);
    HW_PINCTRL_DOE0_SET(nMOS);
    
    while(1)
    {
        HW_PINCTRL_DOUT0_SET(LED);
        delay_us(500000);
        HW_PINCTRL_DOUT0_CLR(LED);
        delay_us(500000);
    }

    return 0;
}
