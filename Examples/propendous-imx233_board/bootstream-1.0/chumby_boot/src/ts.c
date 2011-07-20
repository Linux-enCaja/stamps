#include <utils.h>

// This value needs to be determined by emperical investigation
#define TS_SENSITIVITY 0x800

void ts_init() {
    volatile int *HW_LRADC_CTRL0 = (int *)0x80050000;
    *HW_LRADC_CTRL0 = 0x00180000;
    mdelay(5);
}

int ts_pressed() {
    volatile int *HW_LRADC_CH2   = (int *)0x80050070;
    volatile int *HW_LRADC_CH5   = (int *)0x800500A0;
    volatile int *HW_LRADC_CTRL0 = (int *)0x80050000;

    int lradc_changed = ((*HW_LRADC_CH2)&0x80000000);

    // Kick the LRADC to have it take another sample.
    *HW_LRADC_CTRL0 = 0x00180004;

    // Wait for the top bit to flip, which indicates the sample is ready.
    while(lradc_changed == ((*HW_LRADC_CH2)&0x80000000));

    // We might also look at CH2, which contains the X value.
    return ((*HW_LRADC_CH2)&0x3ffff) < TS_SENSITIVITY;
}

