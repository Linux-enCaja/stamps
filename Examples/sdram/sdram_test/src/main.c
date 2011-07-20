
//#include "setup.h"
#include "platform.h"
#include "utils.h"
#include "types.h"
#include "regspinctrl.h"

#include "hardware_init.h"


u32 sdram_address_error;
u32 sdram_number_words_correct;
u32 SDRAM_word_count;


void blink_led (u32 delay)
{
    while(1)
    {
        HW_PINCTRL_DOUT0_SET(LED);
        delay_us(delay);
        HW_PINCTRL_DOUT0_CLR(LED);
        delay_us(delay);
    }
}

bool test_pattern (u32 pattern)
{
    volatile u32 *pTest;
    u32 value;
    u32 i;
    
    pTest = (u32 *)SDRAM_BASE;
    for (i = 0; i < SDRAM_word_count; i++)
    {
        *pTest = pattern;
        *pTest++;
    }
             
    pTest = (u32 *)SDRAM_BASE;
    for (i = 0; i < SDRAM_word_count; i++)
    {
        value = *pTest;
        if (value != pattern) 
        {
            sdram_address_error = (u32) pTest;
            sdram_number_words_correct = pTest - (u32 *)SDRAM_BASE;
            return FALSE;
        }
        *pTest++;
    }
    
    return TRUE;
}

bool test_count(void)
{
    volatile u32 *pTest;
    u32 value;
    u32 i;
    
    pTest = (u32 *)SDRAM_BASE;
    for (i = 0; i < SDRAM_word_count; i++)
    {
        *pTest = i;
        *pTest++;
    }
             
    pTest = (u32 *)SDRAM_BASE;
    for (i = 0; i < SDRAM_word_count; i++)
    {
        value = *pTest;
        if (value != i) 
        {
            sdram_address_error = (u32) pTest;
            sdram_number_words_correct = pTest - (u32 *)SDRAM_BASE;
            return FALSE;
        }
        *pTest++;
    }
    
    return TRUE;
}


int main(void)
{
    hardware_setup();
     
    /*********************************/
    /* Test SDRAM memory -- 32Mbytes */
    SDRAM_word_count = ((32 * 1024 * 1024) / 4);

    /* test with all bits at 1 */
    /* if fail will quickly blink the LED */
    if (!test_pattern (0xFFFFFFFF))
        blink_led (250000);

    /* test with up count */
    if (!test_count())
        blink_led (250000);
        
    /* SDRAM is ok, blink the LED slowly */
    blink_led (1000000);
    
    return 1;
}
