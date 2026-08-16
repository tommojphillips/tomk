/* tty.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef DRIVER_TTY_H
#define DRIVER_TTY_H

#include <stdint.h>
#include <stddef.h>

/* TTY Initialize */
void tty_init(void);

/* TTY Clear Screen */
void tty_clear_screen(void);

/* TTY Set Cursor Position */
void tty_set_position(size_t row, size_t column);

/* TTY Get Cursor Position */
void tty_get_position(size_t* x, size_t* y);

/* TTY Set Color */
void tty_set_color(uint8_t color);

/* TTY Output entry (char+attribute) at x,y */
void tty_put_entry_at(char c, uint8_t color, size_t x, size_t y);

/* Output character to tty.
c: the character to output. */
void tty_putc(char c);

#endif