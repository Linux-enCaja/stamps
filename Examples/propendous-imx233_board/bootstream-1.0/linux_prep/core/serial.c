/*
 * (c) 2007 Sascha Hauer <s.hauer@pengutronix.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <setup.h>
#include <stmp3xxx.h>
#include <arch/platform.h>

#include "serial.h"


/*
 * Set baud rate. The settings are always 8n1:
 * 8 data bits, no parity, 1 stop bit 
 */
static void serial_setbrg (void)
{
    u32 cr, lcr_h;
    u32 quot;

    // Disable everything
    cr = REG_RD(DBGUART_BASE + UARTDBGCR);
    REG_WR(DBGUART_BASE + UARTDBGCR, 0);

    // Calculate and set baudrate
    quot = CONFIG_DBGUART_CLK * 4 / CONFIG_BAUDRATE;
    REG_WR(DBGUART_BASE + UARTDBGFBRD, quot & 0x3f);
    REG_WR(DBGUART_BASE + UARTDBGIBRD, quot >> 6);

    // Set 8n1 mode, enable FIFOs
    lcr_h = WLEN8 | FEN;
    REG_WR(DBGUART_BASE + UARTDBGLCR_H, lcr_h);

    // Enable Debug UART
    REG_WR(DBGUART_BASE + UARTDBGCR, cr);
}

void serial_init (void)
{
    u32 cr;

    // Set the uart_tx and uart_rx pins to be for the uart, and not e.g.
    // for GPIO.
    HW_PINCTRL_MUXSEL3_CLR((3<<20)|(3<<22));
    HW_PINCTRL_MUXSEL3_SET((2<<20)|(2<<22));


    // Disable UART
    REG_WR(DBGUART_BASE + UARTDBGCR, 0);

    // Mask interrupts
    REG_WR(DBGUART_BASE + UARTDBGIMSC, 0);

    // Set default baudrate
    serial_setbrg();

    // Enable UART
//    cr = DTR | RXE | TXE | RXE | UARTEN;
    cr = REG_RD(DBGUART_BASE + UARTDBGCR);
    cr |= TXE | RXE | UARTEN;
    REG_WR(DBGUART_BASE + UARTDBGCR, cr);


    return;
}

/* Send a character */
void serial_putc (const char c) {
    // Wait for room in TX FIFO.
    while (REG_RD(DBGUART_BASE + UARTDBGFR) & TXFF)
        ;

    // Write the data byte.
    REG_WR(DBGUART_BASE + UARTDBGDR, c);

    if (c == '\n')
        serial_putc('\r');
}

void serial_puts (const char *s) {
    while (*s)
        serial_putc(*s++);
}

/* Test whether a character is in TX buffer */
int serial_tstc (void) {
    /* Check if RX FIFO is not empty */
    return (!(REG_RD(DBGUART_BASE + UARTDBGFR) & RXFE));
}

/* Receive character */
int serial_getc (void)
{
    int data;
    // Wait while TX FIFO is empty
    while (REG_RD(DBGUART_BASE + UARTDBGFR) & RXFE)
        ;

    data = REG_RD(DBGUART_BASE + UARTDBGDR);
    if( data & 0xFFFFFF00 ) {
        // Clear error.
        serial_puts("Clearing serial error...\n");
        REG_WR(DBGUART_BASE + UARTDBGRSR_ECR, 0);
    }

    /* Read data byte */
    return data;
}

static char hex[] = "0123456789abcdef";

void serial_puthex(u32 c) {
    int i;
    serial_puts("0x");
    for(i=7; i>=0; i--)
        serial_putc(hex[(c>>(4*i))&0x0f]);
}



