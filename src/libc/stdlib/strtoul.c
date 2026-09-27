/* strtoul.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stddef.h>

unsigned long strtoul(const char* restrict str, char** restrict end_ptr, int radix) {
    const char* p = str;
    unsigned long value = 0;
    int digit = 0;

    if (p == NULL) {
        return 0;
    }

    if (*p == '\0') {
        return 0;
    }

    if (radix == 0) {
        if (*p == '0') {
            p++;
            if (*p == '\0') {
                return 0;
            }
            else if (*p == 'x' || *p == 'X') {
                p++;
                radix = 16;
            }
            else if (*p == 'b' || *p == 'B') {
                p++;
                radix = 2;
            }
            else {
                radix = 8;
            }
        }
        else {
            radix = 10;
        }
    }

    if (radix < 2 || radix > 36) {
        return 0;
    }

    while (*p) {
        if (*p >= '0' && *p <= '9') {
            digit = *p - '0';
        }
        else if (*p >= 'a' && *p <= 'z') {
            digit = *p - 'a' + 10;
        }
        else if (*p >= 'A' && *p <= 'Z') {
            digit = *p - 'A' + 10;
        }
        else {
            break;
        }
        
        if (digit >= radix) {
            break;
        }

        value = value * radix + digit;
        p++;
    }

    if (end_ptr) {
        *end_ptr = (char*)p;
    }

    return value;
}
unsigned long long strtoull(const char* restrict str, char** restrict end_ptr, int radix) {
    const char* p = str;
    unsigned long long value = 0;
    int digit = 0;

    if (p == NULL) {
        return 0;
    }

    if (*p == '\0') {
        return 0;
    }

    if (radix == 0) {
        if (*p == '0') {
            p++;
            if (*p == '\0') {
                return 0;
            }
            else if (*p == 'x' || *p == 'X') {
                p++;
                radix = 16;
            }
            else if (*p == 'b' || *p == 'B') {
                p++;
                radix = 2;
            }
            else {
                radix = 8;
            }
        }
        else {
            radix = 10;
        }
    }

    if (radix < 2 || radix > 36) {
        return 0;
    }

    while (*p) {
        if (*p >= '0' && *p <= '9') {
            digit = *p - '0';
        }
        else if (*p >= 'a' && *p <= 'z') {
            digit = *p - 'a' + 10;
        }
        else if (*p >= 'A' && *p <= 'Z') {
            digit = *p - 'A' + 10;
        }
        else {
            break;
        }
        
        if (digit >= radix) {
            break;
        }

        value = value * radix + digit;
        p++;
    }

    if (end_ptr) {
        *end_ptr = (char*)p;
    }

    return value;
}

long strtol(const char* restrict str, char** restrict end_ptr, int radix) {
    const char* p = str;
    long value = 0;
    int digit = 0;
    int negative = 0;

    if (p == NULL) {
        return 0;
    }

    if (*p == '\0') {
        return 0;
    }

    if (*p == '+' || *p == '-') {
        if (*p == '-') {
            negative = 1;
        }

        p++;

        if (*p == '\0') {
            if (end_ptr) {
                *end_ptr = (char *)str;
            }
            return 0;
        }
    }
    
    if (radix == 0) {
        if (*p == '0') {
            p++;
            if (*p == '\0') {
                return 0;
            }
            else if (*p == 'x' || *p == 'X') {
                p++;
                radix = 16;
            }
            else if (*p == 'b' || *p == 'B') {
                p++;
                radix = 2;
            }
            else {
                radix = 8;
            }
        }
        else {
            radix = 10;
        }
    }

    if (radix < 2 || radix > 36) {
        return 0;
    }

    while (*p) {
        if (*p >= '0' && *p <= '9') {
            digit = *p - '0';
        }
        else if (*p >= 'a' && *p <= 'z') {
            digit = *p - 'a' + 10;
        }
        else if (*p >= 'A' && *p <= 'Z') {
            digit = *p - 'A' + 10;
        }
        else {
            break;
        }
        
        if (digit >= radix) {
            break;
        }

        value = value * radix + digit;
        p++;
    }

    if (negative) {
        value = -value;
    }
    
    if (end_ptr) {
        *end_ptr = (char*)p;
    }

    return value;
}
long long strtoll(const char* restrict str, char** restrict end_ptr, int radix) {
    const char* p = str;
    long long value = 0;
    int digit = 0;
    int negative = 0;

    if (p == NULL) {
        return 0;
    }

    if (*p == '\0') {
        return 0;
    }

    if (*p == '+' || *p == '-') {
        if (*p == '-') {
            negative = 1;
        }

        p++;

        if (*p == '\0') {
            if (end_ptr) {
                *end_ptr = (char *)str;
            }
            return 0;
        }
    }
    
    if (radix == 0) {
        if (*p == '0') {
            p++;
            if (*p == '\0') {
                return 0;
            }
            else if (*p == 'x' || *p == 'X') {
                p++;
                radix = 16;
            }
            else if (*p == 'b' || *p == 'B') {
                p++;
                radix = 2;
            }
            else {
                radix = 8;
            }
        }
        else {
            radix = 10;
        }
    }

    if (radix < 2 || radix > 36) {
        return 0;
    }

    while (*p) {
        if (*p >= '0' && *p <= '9') {
            digit = *p - '0';
        }
        else if (*p >= 'a' && *p <= 'z') {
            digit = *p - 'a' + 10;
        }
        else if (*p >= 'A' && *p <= 'Z') {
            digit = *p - 'A' + 10;
        }
        else {
            break;
        }
        
        if (digit >= radix) {
            break;
        }

        value = value * radix + digit;
        p++;
    }

    if (negative) {
        value = -value;
    }
    
    if (end_ptr) {
        *end_ptr = (char*)p;
    }

    return value;
}

int atoi(const char* str) {
    return (int)strtol(str, NULL, 10);
}
long atol(const char* str) {
    return strtol(str, NULL, 10);    
}
long long atoll(const char* str) {
    return strtoll(str, NULL, 10);
}
