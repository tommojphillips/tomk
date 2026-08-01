/* libc/stdio/printf.c */

#include <stdarg.h>
#include <string.h>
#include <tty.h>

#define MAX_BUFFER 32

static void pr_int(int64_t value, int width, int precision, char pad, int left) {
    if (value < 0) {
        tty_putc('-');
        tty_putn((uint64_t)(-value), 10, 0, width, precision, pad, left);
    } else {
        tty_putn((uint64_t)value, 10, 0, width, precision, pad, left);
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

        int left = 0;
        char pad = ' ';
        int width = 0;
        int precision = -1;
        int long_long = 0;

        if (*fmt == '-') {
            left = 1;
            fmt++;
        }
        else if (*fmt == '0') {
            pad = '0';
            fmt++;
        }

        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        if (*fmt == '.') {
            fmt++;

            precision = 0;

            while (*fmt >= '0' && *fmt <= '9') {
                precision = precision * 10 + (*fmt - '0');
                fmt++;
            }

            pad = ' ';
        }
        
        if (*fmt == 'l') {
            fmt++;

            if (*fmt == 'l') {
                fmt++;
                long_long = 1;
            }
        }
        
        switch (*fmt) {
            
            case 'c': {
                char c = (char)va_arg(args, int);                
                tty_putc(c);
                count++;
            } break;

            case 's': {
                const char* s = va_arg(args, const char*);
                if (s == NULL) {
                    s = "(null)";
                }
                
                size_t len = strlen(s);
                if (precision >= 0 && len > (size_t)precision) {
                    len = precision;
                }
                
                if (!left) {
                    while ((size_t)width > len) {
                        tty_putc(' ');
                        width--;
                        count++;
                    }
                }

                for (size_t i = 0; i < len; i++) {
                    tty_putc(s[i]);
                    count++;
                }

                if (left) {
                    while ((size_t)width > len) {
                        tty_putc(' ');
                        width--;
                        count++;
                    }
                }
                break;
            }

            case 'd':
            case 'i': {
                if (long_long) {
                    int64_t v = va_arg(args, int64_t);
                    pr_int(v, width, precision, pad, left);
                    count++;
                }
                else {
                    int32_t v = va_arg(args, int32_t);
                    pr_int(v, width, precision, pad, left);
                    count++;
                }
                count++;
            } break;

            case 'u': {
                if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    tty_putn(v, 10, 0, width, precision, pad, left);
                    count++;
                }
                else {
                    uint32_t v = va_arg(args, uint32_t);
                    tty_putn(v, 10, 0, width, precision, pad, left);
                    count++;
                }
            } break;

            case 'x': {
                if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    tty_putn(v, 16, 1, width, precision, pad, left);
                    count++;
                }
                else {
                    uint32_t v = va_arg(args, uint32_t);
                    tty_putn(v, 16, 1, width, precision, pad, left);
                    count++;
                }
            } break;

            case 'X': {
                if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    tty_putn(v, 16, 0, width, precision, pad, left);
                    count++;
                }
                else {
                    uint32_t v = va_arg(args, uint32_t);
                    tty_putn(v, 16, 0, width, precision, pad, left);
                    count++;
                }
            } break;

            case 'o': {
                if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    tty_putn(v, 8, 0, width, precision, pad, left);
                    count++;
                }
                else {
                    uint32_t v = va_arg(args, uint32_t);
                    tty_putn(v, 8, 0, width, precision, pad, left);
                    count++;
                }
            } break;

            case 'b': {
                if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    tty_putn(v, 2, 0, width, precision, pad, left);
                    count++;
                }
                else {
                    uint32_t v = va_arg(args, uint32_t);
                    tty_putn(v, 2, 0, width, precision, pad, left);
                    count++;
                }
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
