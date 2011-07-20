/*
 * Low-level LRADC keypad driver
 *
 * Vladislav Buzov <vbuzov@embeddedalley.com>
 *
 * Copyright 2008 SigmaTel, Inc
 * Copyright 2008 Embedded Alley Solutions, Inc
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2.  This program  is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */
#include <stmp3xxx.h>
#include <keys.h>
#include <init.h>
#include <serial.h>
#include <setup.h>

#define TARGET_VDDIO_LRADC_VAL  3754
#define LRADC_NOISE_MARGIN  100


static u32 chumby_bend_read_button() {
    u32 value = !(*(u32 *)0x80018610&0x40000000);
    if(value)
        return KEY4; // Magic values.  This is almost-but-not-quite modular.
    return NO_KEY;
}


static void chumby_bend_hwinit() {
    // Switch PWM4 to be a GPIO.  Bank 1, Pin 30.
    //HW_PINCTRL_MUXSEL3_SET=0x30000000
    *(u32 *)0x80018134=0x30000000;

    // Switch Bank 1, Pin 30 to have no pullup.
    //HW_PINCTRL_PULL1_CLR=0x40000000
    *(u32 *)0x80018418=0x40000000;

    // Switch the pin to be input-only.
    //HW_PINCTRL_DOE1_CLR=0x04000000
    *(u32 *)0x80018718=0x40000000;
}

struct keys_driver chumby_bend_driver = {
    .id         = KEYS_DRV_GPIO,
    .get_keys   = chumby_bend_read_button,
};

static void chumby_bend_init (void)
{
    chumby_bend_hwinit();
    set_keys_driver(&chumby_bend_driver);
}

hw_initcall(chumby_bend_init);
