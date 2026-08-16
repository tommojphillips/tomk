/* libc/string/strcmp.c */

#include <stddef.h>
#include <string.h>

int strcmp(const char* restrict s1, const char* restrict s2) {
    for (;;) {
        unsigned char c1 = (unsigned char)*s1++;
        unsigned char c2 = (unsigned char)*s2++;

        if (c1 != c2) {
            return c1 < c2 ? -1 : 1;
        }

        if (c1 == '\0') {
            return 0;
        }
    }
}
int strncmp(const char* restrict s1, const char* restrict s2, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        unsigned char c1 = (unsigned char)s1[i];
        unsigned char c2 = (unsigned char)s2[i];

        if (c1 != c2) {
            return c1 < c2 ? -1 : 1;
        }

        if (c1 == '\0') {
            break;
        }
    }

    return 0;
}
