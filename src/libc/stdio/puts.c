/* libc/stdio/puts.c */

#include <stdio.h>
#include <tty.h>

int puts(const char* s) {
    if (s == NULL) {
        return EOF;
    }
    tty_puts(s);
    return 1;
}

