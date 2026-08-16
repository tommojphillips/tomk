/* libc/stdio/puts.c */

#include <stdio.h>
#include <tty.h>

int puts(const char* s) {
    if (s == NULL) {
        return EOF;
    }
    
    while (*s) {
		tty_putc(*s);
        s++;        
    }
    return 1;
}
