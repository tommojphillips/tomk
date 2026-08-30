/* libc/stdio/printf.c */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <tty.h>
#include <uart.h>

#define MAX_BUFFER_SIZE 64

#define max(a,b) ((a) > (b) ? (a) : (b))

typedef struct printf_output_t {
    void (*putc)(void*, char);
    void* userparam;
    size_t count;
} printf_output_t;

typedef struct {
    char* buffer;
    size_t length;
    size_t count;
} string_output_t;

static const char* hex_digits_u = "0123456789ABCDEF";
static const char* hex_digits_l = "0123456789abcdef";
static char buffer[MAX_BUFFER_SIZE];

static void serial_output(void* userparam, char c) {
    (void)userparam;
    serial_write(c);
}
static void tty_output(void* userparam, char c) {
    (void)userparam;
    tty_putc(c);
}
static void string_output(void* userparam, char c) {
    string_output_t* out = (string_output_t*)userparam;
    if (out->count < out->length) {
        out->buffer[out->count] = c;
    }
    out->count++;
}
static void output_char(printf_output_t* out, char c) {
    out->putc(out->userparam, c);
    out->count++;
}

static void pr_number(printf_output_t* out, uint64_t number, int is_signed, int base, int lower, int width, int precision, char pad, int left) {
    size_t i = 0;
    int negative = 0;
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

    if (is_signed && (int64_t)number < 0) {
        negative = 1;
        number = (uint64_t)(-(int64_t)(number + 1)) + 1;
    }

	if (number == 0) {
		buffer[i++] = '0';
	}
	else {
        uint64_t j = number;
        switch (base) {
            case 2:
                while (j && i < MAX_BUFFER_SIZE) {
                    buffer[i++] = hex_digits[j & 0x1];
                    j >>= 1;
                }
                break;

            case 8:
                while (j && i < MAX_BUFFER_SIZE) {
                    buffer[i++] = hex_digits[j & 0x7];
                    j >>= 3;
                }
                break;

            case 16:
                while (j && i < MAX_BUFFER_SIZE) {
                    buffer[i++] = hex_digits[j & 0xF];
                    j >>= 4;
                }
                break;

            case 10:
            default:
                while (j && i < MAX_BUFFER_SIZE) {
                    buffer[i++] = hex_digits[j % base];
                    j /= base;
                }
                break;
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
		spaces = max(width - digits - zeros - negative, 0);
	} else {
		if (pad == '0') {
			zeros = max(width - digits - negative, 0);
			spaces = 0;
		} else {
			spaces = max(width - digits - negative, 0);
			zeros = 0;
		}
	}

	if (!left) {
		while (spaces > 0) {
			spaces--;
			output_char(out, ' ');
		}
	}

    if (negative) {
        output_char(out, '-');
    }

	while (zeros > 0) {
        zeros--;
		output_char(out, '0');
	}

	while (digits > 0) {
        digits--;
		output_char(out, buffer[digits]);
	}

	if (left) {
		while (spaces > 0) {
			spaces--;
			output_char(out, ' ');
		}
	}
}
static void pr_uint(printf_output_t* out, uint64_t number, int base, int lower, int width, int precision, char pad, int left) {
    pr_number(out, number, 0, base, lower, width, precision, pad, left);
}
static void pr_int(printf_output_t* out, int64_t number, int width, int precision, char pad, int left) {
    pr_number(out, (uint64_t)number, 1, 10, 0, width, precision, pad, left);
}
static void pr_string(printf_output_t* out, const char* s, int width, int precision, int left) {
    if (s == NULL) {
        s = "(null)";
    }
    
    size_t len = strlen(s);
    if (precision >= 0 && len > (size_t)precision) {
        len = precision;
    }
    
    if (!left) {
        while ((size_t)width > len) {
            output_char(out, ' ');
            width--;
        }
    }

    for (size_t i = 0; i < len; i++) {
        output_char(out, s[i]);
    }

    if (left) {
        while ((size_t)width > len) {
            output_char(out, ' ');
            width--;
        }
    }
}

static int parse_number(const char* restrict* fmt) {
    int number = 0;
    while (**fmt >= '0' && **fmt <= '9') {
        number = number * 10 + (**fmt - '0');
        (*fmt)++;
    }
    return number;
}

void vformat(printf_output_t* out, const char* restrict fmt, va_list args) {
    int left = 0;
    char pad = 0;
    int width = 0;
    int precision = 0;
    int long_long = 0;
    int lower = 0;

    while (*fmt) {
        if (*fmt != '%') {
            output_char(out, *fmt);
            fmt++;
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
                output_char(out, c);
            } break;

            case 's': {
                const char* s = va_arg(args, const char*);
                pr_string(out, s, width, precision, left);                
                break;
            }

            case 'd':
            case 'i': {
                if (long_long) {
                    int64_t v = va_arg(args, int64_t);
                    pr_int(out, v, width, precision, pad, left);
                }
                else {
                    int32_t v = va_arg(args, int32_t);
                    pr_int(out, v, width, precision, pad, left);
                }
            } break;

            case 'u': {
                if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    pr_uint(out, v, 10, lower, width, precision, pad, left);
                }
                else {
                    uint32_t v = va_arg(args, uint32_t);
                    pr_uint(out, v, 10, lower, width, precision, pad, left);
                }
            } break;
            
            case 'x': 
            case 'X': {
                if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    pr_uint(out, v, 16, lower, width, precision, pad, left);
                }
                else {
                    uint32_t v = va_arg(args, uint32_t);
                    pr_uint(out, v, 16, lower, width, precision, pad, left);
                }
            } break;

            case 'o': {
                if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    pr_uint(out, v, 8, lower, width, precision, pad, left);
                }
                else {
                    uint32_t v = va_arg(args, uint32_t);
                    pr_uint(out, v, 8, lower, width, precision, pad, left);
                }
            } break;

            case 'b': {
                if (long_long) {
                    uint64_t v = va_arg(args, uint64_t);
                    pr_uint(out, v, 2, lower, width, precision, pad, left);
                }
                else {
                    uint32_t v = va_arg(args, uint32_t);
                    pr_uint(out, v, 2, lower, width, precision, pad, left);
                }
            } break;

            case '%': {
                output_char(out, '%');
            } break;

            default: {
                output_char(out, '%');
                output_char(out, *fmt);
            } break;
        }

        fmt++;
    }
}

int vsprintf(char* restrict s, const char* restrict fmt, va_list args) {
    string_output_t str = { .buffer = s, .length = SIZE_MAX };
    printf_output_t out = { .putc = string_output, .userparam = &str };
    vformat(&out, fmt, args);
    s[out.count] = '\0';
    return out.count;
}
int vsnprintf(char* restrict s, size_t len, const char* restrict fmt, va_list args) {
    string_output_t str = { .buffer = s, .length = len };
    printf_output_t out = { .putc = string_output, .userparam = &str };
    vformat(&out, fmt, args);
    if (len != 0) {
        if ((size_t)out.count < len) {
            s[out.count] = '\0';
        }
        else {
            s[len - 1] = '\0';
        }
    }
    return out.count;
}
int vprintf(const char* restrict fmt, va_list args) {
    printf_output_t out = { .putc = tty_output };
    vformat(&out, fmt, args);
    return out.count;
}

int sprintf(char* restrict buffer, const char* restrict fmt, ...) {
    va_list args;    
    va_start(args, fmt);
    int count = vsprintf(buffer, fmt, args);
    va_end(args);
    return count;
}
int snprintf(char* restrict buffer, size_t buffer_size, const char* restrict fmt, ...) {
    va_list args;    
    va_start(args, fmt);
    int count = vsnprintf(buffer, buffer_size, fmt, args);
    va_end(args);
    return count;
}
int printf(const char* restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int count = vprintf(fmt, args);
    va_end(args);
    return count;
}

int vfprintf(print_dest_t stream, const char* restrict fmt, va_list args) {
    printf_output_t out;

    switch (stream) {
        case STDIO:
            out.putc = tty_output;
            break;
        case SERIAL:
            out.putc = serial_output;
            break;
        default:
            return -1;
    }

    out.userparam = NULL;
    out.count = 0;

    vformat(&out, fmt, args);
    return (int)out.count;
}
int fprintf(print_dest_t stream, const char* restrict fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int count = vfprintf(stream, fmt, args);
    va_end(args);
    return count;
}
