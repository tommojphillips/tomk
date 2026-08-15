/* libc/stdio/printf.c */

#include <stdarg.h>
#include <string.h>
#include <tty.h>

static int pr_int(int64_t value, int width, int precision, char pad, int left) {
    int count = 0;
    if (value < 0) {
        tty_putc('-');
        count++;
        count += tty_putn((uint64_t)(-value), 10, 0, width, precision, pad, left);
    } else {
        count = tty_putn((uint64_t)value, 10, 0, width, precision, pad, left);
    }
    return count;
}
static int pr_uint(uint64_t value, int base, int lower, int width, int precision, char pad, int left) {    
    return tty_putn(value, base, lower, width, precision, pad, left);
}
static int pr_string(const char* s, int width, int precision, int left) {
    int count = 0;

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

    return count;
}
static int parse_number(const char* restrict* fmt) {
    int number = 0;
    while (**fmt >= '0' && **fmt <= '9') {
        number = number * 10 + (**fmt - '0');
        (*fmt)++;
    }
    return number;
}

int vprintf(const char* restrict fmt, va_list args) {
    size_t count;
    int left;
    char pad;
    int width;
    int precision;
    int long_long;
    int lower;

    count = 0;
    while (*fmt) {
        if (*fmt != '%') {
            tty_putc(*fmt);
            fmt++;
            count++;
            continue;
        }

        fmt++;

        left = 0;
        pad = ' ';
        width = 0;
        precision = -1;
        long_long = 0;
        lower = 0;

        if (*fmt == '-') {
            fmt++;
            left = 1;
        }
        else if (*fmt == '0') {
            fmt++;
            pad = '0';
        }

        width = parse_number(&fmt);

        if (*fmt == '.') {
            fmt++;

            precision = parse_number(&fmt);
            pad = ' ';
        }
        
        if (*fmt == 'l') {
            fmt++;

            if (*fmt == 'l') {
                fmt++;
                long_long = 1;
            }
        }

        if (*fmt == 'x') {
            lower = 1;
        }
        
        switch (*fmt) {
            
            case 'c': {
                char c = (char)va_arg(args, int);                
                tty_putc(c);
                count++;
            } break;

            case 's': {
                const char* s = va_arg(args, const char*);
                count += pr_string(s, width, precision, left);                
                break;
            }

            case 'd':
            case 'i': {
                if (long_long) {
                    int64_t v = va_arg(args, int64_t);
                    count += pr_int(v, width, precision, pad, left);
                }
                else {
                    int32_t v = va_arg(args, int32_t);
                    count += pr_int(v, width, precision, pad, left);
                }
            } break;

            case 'u': {
                if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    count += pr_uint(v, 10, lower, width, precision, pad, left);
                }
                else {
                    uint32_t v = va_arg(args, uint32_t);
                    count += pr_uint(v, 10, lower, width, precision, pad, left);
                }
            } break;
            
            case 'x': 
            case 'X': {
                if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    count += pr_uint(v, 16, lower, width, precision, pad, left);
                }
                else {
                    uint32_t v = va_arg(args, uint32_t);
                    count += pr_uint(v, 16, lower, width, precision, pad, left);
                }
            } break;

            case 'o': {
                if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    count += pr_uint(v, 8, lower, width, precision, pad, left);
                }
                else {
                    uint32_t v = va_arg(args, uint32_t);
                    count += pr_uint(v, 8, lower, width, precision, pad, left);
                }
            } break;

            case 'b': {
                if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    count += pr_uint(v, 2, lower, width, precision, pad, left);
                }
                else {
                    uint32_t v = va_arg(args, uint32_t);
                    count += pr_uint(v, 2, lower, width, precision, pad, left);
                }
            } break;

            case '%': {
                tty_putc('%');
                count += 1;
            } break;

            default: {
                tty_putc('%');
                tty_putc(*fmt);
                count += 2;
            } break;
        }

        fmt++;
    }
    return count;
}
int printf(const char* restrict fmt, ...) {
    va_list args;
    size_t count;
    
    va_start(args, fmt);
    count = vprintf(fmt, args);
    va_end(args);
    return count;
}
