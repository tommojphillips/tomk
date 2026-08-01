/* libc/stdio/putchar.c */

#include <tty.h>

int putchar(int ch) {
    tty_putc((char)ch);
    return ch;
}
