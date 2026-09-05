/* libc/stdio/getchar.c */

#include <stdio.h>
#include <ps2.h>
#include <uart.h>

int getchar(void) {
#ifdef LIBK
    int ch = ps2_getchar();
    if (ch == 0) {
        ch = serial_read();
    }    
    if (ch == 0) {
        return EOF;
    }
    return ch;
#else
    return 0;
#endif
}
