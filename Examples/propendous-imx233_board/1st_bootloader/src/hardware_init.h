/* ========================================================================== */
/*                                                                            */
/*   hardware_init.h                                                          */
/*   (c) 2001 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#ifndef _HARDWARE_INIT_H
#define _HARDWARE_INIT_H

/* LED pin lives in PWM, bank 1, pin 28 */
#define LED (1 << 28)

void delay_us (int us);
void hardware_setup(void);

#endif
