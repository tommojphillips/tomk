/* libc/string/strspn.c */

#include <stdint.h>
#include <stddef.h>

size_t strspn(const char* dest, const char* src) {
    size_t i = 0;
    size_t j = 0;

    if (dest == NULL || src == NULL) {
        return 0;
    }

    while (dest[i] != '\0') {
        j = 0;

        while (src[j] != '\0') {
            if (src[j] == dest[i]) {
                break;
            }
            
            j++;
        }

        if (src[j] == '\0') {
            return i;
        }

        i++;
    }
    
    return i;
}

size_t strcspn(const char* dest, const char* src) {
    size_t i = 0;
    size_t j = 0;

    if (dest == NULL || src == NULL) {
        return 0;
    }

    while (dest[i] != '\0') {
        j = 0;

        while (src[j] != '\0') {
            if (src[j] == dest[i]) {
                return i;
            }

            j++;
        }

        i++;
    }

    return i;
}
