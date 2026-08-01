/* driver/tty.h */

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

/* TTY Set Color */
void tty_set_color(uint8_t color);

/* TTY Output entry (char+attribute) at x,y */
void tty_put_entry_at(char c, uint8_t color, size_t x, size_t y);

/* Output character to tty.
c: the character to output. */
void tty_putc(char c);

/* Output character(s) to tty.
data: the data to output.
size: the size of the data. */
void tty_putd(const char* data, size_t size);

/* Output Null-Terminated string to tty.
s: the string to output. */
void tty_puts(const char* s);

/* Output number in base to tty.
number: the number to output.
base:   the base to output the number in.
lower:  1 = lower case; 0 = upper case
2 (Binary), 8 (Octal), 10 (Decimal), 16 (Hexadecimal). */
void tty_putn(uint32_t number, int base, int lower);

#endif