#include "platform.h"
#include "utils.h"
#include "types.h"
#include "regspinctrl.h"

#include "hardware_init.h"

void blink_led (u32 delay)
{
    while(1)
    {
        HW_PINCTRL_DOUT1_SET(LED);
        delay_us(delay);
        HW_PINCTRL_DOUT1_CLR(LED);
        delay_us(delay);
    }
}

int main(void)
{
    hardware_setup();
     
    while (1)
      {
    blink_led (250000);
    blink_led (1000000);
      }
    
    return 1;
}
