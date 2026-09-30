/* libc/string/memcpy.c */

#include <stddef.h>

int memcmp(const void* lhs, const void* rhs, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (((unsigned char*)lhs)[i] != ((unsigned char*)rhs)[i]) {
            return 1; /* not equal */
        }
    }

    return 0; /* equal */
}
