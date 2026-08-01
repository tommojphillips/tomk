
#include <stdio.h>
#include <ps2.h>

int getchar(void) {
    int ch = ps2_getchar();
    if (ch == 0) {
        return EOF;
    }
    return ch;
}
