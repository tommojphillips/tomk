/* driver/tty.c */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <tty.h>
#include <vga.h>
#include <string.h>

#define TTY_MAX_BUFFER_SIZE 32

static size_t tty_row;           /* current row */
static size_t tty_column;        /* current column*/
static uint8_t tty_color;        /* current color */
static uint16_t* vga_buffer;     /* video buffer */
static char tty_buffer[TTY_MAX_BUFFER_SIZE];

static const char* hex_digits_u = "0123456789ABCDEF";
static const char* hex_digits_l = "0123456789abcdef";

void move_cursor(size_t x, size_t y) {
    size_t pos = (y * VGA_WIDTH) + x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, pos & 0xFF);

    outb(0x3D4, 0x0E);
    outb(0x3D5, (pos >> 8) & 0xFF);
}

void tty_init(void) {
	tty_row = 0;
	tty_column = 0;
	tty_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	vga_buffer = (uint16_t*)VGA_MEMORY;
	memset(tty_buffer, 0, TTY_MAX_BUFFER_SIZE);

	tty_clear_screen();
}

void tty_clear_screen(void) {	
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			vga_buffer[index] = vga_entry(' ', tty_color);
		}
	}
}

void tty_set_position(size_t row, size_t column) {
	tty_row = row;
	if (tty_row > VGA_HEIGHT) {
		tty_row = VGA_HEIGHT;
	}

	tty_column = column;
	if (tty_column > VGA_WIDTH) {
		tty_column = VGA_WIDTH;
	}
	move_cursor(tty_column, tty_row);
}

void tty_set_color(uint8_t color) {
	tty_color = color & 0xF;
}

void tty_put_entry_at(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	vga_buffer[index] = vga_entry(c, color);
}

void tty_putc(char c) {
	bool r = (c == '\r' || c == '\n' || c == '\t' || c == '\b');
	
	if (!r) {
		tty_put_entry_at(c, tty_color, tty_column, tty_row);
	}
	
	if (++tty_column == VGA_WIDTH || c == '\n') {
		tty_column = 0;
		if (++tty_row == VGA_HEIGHT) {
			tty_row = 0;
		}
	}
	move_cursor(tty_column, tty_row);
}

void tty_putd(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++) {
		tty_putc(data[i]);
	}
}

void tty_puts(const char* s) {
	tty_putd(s, strlen(s));
}

void tty_putn(uint32_t number, int base, int lower) {
    size_t i = 0;
	const char* hex_digits;

	if (number == 0) {		
		tty_putc('0');
	}

	if (lower) {
		hex_digits = hex_digits_l;
	}
	else {
	    hex_digits = hex_digits_u;
	}

	if (base != 2 && base != 8 && base != 10 && base != 16) {
		base = 10;
	}

    while (number && i < TTY_MAX_BUFFER_SIZE) {
		tty_buffer[i++] = hex_digits[number % base];
        number /= base;
    }

	while (i--) {
		tty_putc(tty_buffer[i]);
	}
}
