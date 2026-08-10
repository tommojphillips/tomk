/* tty.c */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <tty.h>
#include <vga.h>
#include <i86.h>

#define TTY_MAX_BUFFER_SIZE 64

#define max(a,b) ((a) > (b) ? (a) : (b))

static size_t tty_row;           /* current row */
static size_t tty_column;        /* current column*/
static uint8_t tty_color;        /* current color */
static uint16_t* vga_buffer;     /* video buffer */
static char tty_buffer[TTY_MAX_BUFFER_SIZE];

static const char* hex_digits_u = "0123456789ABCDEF";
static const char* hex_digits_l = "0123456789abcdef";

static void cursor_enable(void);
static void cursor_move(size_t x, size_t y);
static void cursor_scroll(size_t x, size_t y);
static void pute(char c, uint8_t color, size_t x, size_t y);

void tty_init(void) {
	tty_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	vga_buffer = (uint16_t*)VGA_MEMORY;
	memset(tty_buffer, 0, TTY_MAX_BUFFER_SIZE);

	cursor_enable();
	tty_clear_screen();
}

void tty_clear_screen(void) {	
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			vga_buffer[index] = vga_entry(' ', tty_color);
		}
	}
	tty_row = 0;
	tty_column = 0;
	cursor_move(0, 0);
	cursor_scroll(0, 0);
}
void tty_clear_line(size_t x, size_t y) {
	for (size_t i = x; i < VGA_WIDTH; ++i) {
		pute(' ', tty_color, i, y);
	}
}
void tty_set_position(size_t x, size_t y) {
	tty_row = y;
	if (tty_row >= VGA_HEIGHT) {
		tty_row = VGA_HEIGHT - 1;
	}

	tty_column = x;
	if (tty_column >= VGA_WIDTH) {
		tty_column = VGA_WIDTH - 1;
	}
	cursor_move(tty_column, tty_row);
}
void tty_get_position(size_t* x, size_t* y) {
	*x = tty_column;
	*y = tty_row;
}
void tty_set_color(uint8_t color) {
	tty_color = color & 0xF;
}

void tty_putc(char c) {
	bool r = (c == '\r' || c == '\n' || c == '\t' || c == '\b');
	bool u = false;

	if (!r) {
		pute(c, tty_color, tty_column, tty_row);
	}
	
	if (c == '\n' && tty_column < VGA_WIDTH) {
		tty_clear_line(tty_column, tty_row);
		u = true;
	}
	else if (c == '\n') {
		u = true;
	}
	else if (++tty_column >= VGA_WIDTH) {
		u = true;
	}

	if (u) {
		tty_column = 0;
		if (++tty_row >= VGA_HEIGHT) {
			cursor_scroll(tty_column, tty_row-VGA_HEIGHT+1);
		}
	}
	cursor_move(tty_column, tty_row);
}
void tty_putd(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++) {
		tty_putc(data[i]);
	}
}
void tty_puts(const char* s) {
	tty_putd(s, strlen(s));
}
int tty_putn(uint64_t number, int base, int lower, int width, int precision, char pad, int left) {
    size_t i = 0;
	int count = 0;
	const char* hex_digits;

	if (lower) {
		hex_digits = hex_digits_l;
	}
	else {
	    hex_digits = hex_digits_u;
	}

	if (base != 2 && base != 8 && base != 10 && base != 16) {
		base = 10;
	}

	if (number == 0) {
		tty_buffer[i++] = '0';
	}
	else {
		size_t j = number;
		while (j && i < TTY_MAX_BUFFER_SIZE) {
			tty_buffer[i++] = hex_digits[j % base];
			j /= base;
		}
	}

	int digits = (int)i;
	int zeros = 0;
	int spaces = 0;

	if (precision == 0 && number == 0) {
		digits = 0;
	}

	if (precision >= 0) {
		zeros = max(precision - digits, 0);
		spaces = max(width - digits - zeros, 0);
	} else {
		if (pad == '0') {
			zeros = max(width - digits, 0);
			spaces = 0;
		} else {
			spaces = max(width - digits, 0);
			zeros = 0;
		}
	}

	if (!left) {
		while (spaces > 0) {
			tty_putc(' ');
			spaces--;
			count++;
		}
	}

	while (zeros--) {
		tty_putc('0');
		count++;
	}

	while (digits--) {
		tty_putc(tty_buffer[digits]);
		count++;
	}

	if (left) {
		while (spaces > 0) {
			tty_putc(' ');
			count++;
			spaces--;
		}
	}
	return count;
}

static void cursor_enable(void) {
    outb(0x3D4, 0x0A);
    uint8_t start = inb(0x3D5);

    start &= 0x1F;       /* cursor start scanline */
    start &= ~(1 << 5);  /* enable cursor */

    outb(0x3D4, 0x0A);
    outb(0x3D5, start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, 0x0F);   /* cursor end scanline */
}
static void cursor_move(size_t x, size_t y) {
    size_t pos = (y * VGA_WIDTH) + x;

    outb(0x3D4, 0x0F);              /* select register */
    outb(0x3D5, pos & 0xFF);        /* low byte */

    outb(0x3D4, 0x0E);              /* select register */
    outb(0x3D5, (pos >> 8) & 0xFF); /* high byte */
}
static void cursor_scroll(size_t x, size_t y) {
    size_t pos = (y * VGA_WIDTH) + x;

    outb(0x3D4, 0x0D);              /* select register */
    outb(0x3D5, pos & 0xFF);        /* low byte */

    outb(0x3D4, 0x0C);              /* select register */
    outb(0x3D5, (pos >> 8) & 0xFF); /* high byte */
}
static void pute(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	vga_buffer[index] = vga_entry(c, color);
}
