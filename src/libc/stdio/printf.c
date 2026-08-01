/* libc/stdio/printf.c */

#include <stdarg.h>
#include <tty.h>

#define MAX_BUFFER 32

static void pr_int(int value) {
    if (value < 0) {
        tty_putc('-');
        tty_putn((unsigned int)(-value), 10, 0);
    } else {
        tty_putn((unsigned int)value, 10, 0);
    }
}

int printf(const char* restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t count = 0; 
    while (*fmt) {
        if (*fmt != '%') {
            tty_putc(*fmt++);
            count++;
            continue;
        }

        fmt++;

        switch (*fmt) {
            
            case 'c': {
                char c = (char)va_arg(args, int);                
                tty_putc(c);
                count++;
            } break;

            case 's': {
                const char* s = va_arg(args, const char*);
                tty_puts(s ? s : "(null)");
                count++;
                break;
            }

            case 'd':
            case 'i': {
                int32_t v = va_arg(args, signed int);
                pr_int(v);
                count++;
            } break;

            case 'u': {
                uint32_t v = va_arg(args, unsigned int);
                tty_putn(v, 10, 0);
                count++;
            } break;

            case 'x': {
                uint32_t v = va_arg(args, unsigned int);
                tty_putn(v, 16, 1);
                count++;
            } break;

            case 'X': {
                uint32_t v = va_arg(args, unsigned int);
                tty_putn(v, 16, 0);
                count++;
            } break;

            case 'o': {
                uint32_t v = va_arg(args, unsigned int);
                tty_putn(v, 8, 0);
                count++;
            } break;

            case 'b': {
                uint32_t v = va_arg(args, unsigned int);
                tty_putn(v, 2, 0);
                count++;
            } break;

            default: {
                tty_putc('%');
                tty_putc(*fmt);
                count++;
                count++;
            } break;
        }

        fmt++;
    }

    va_end(args);
    return 0;
}
