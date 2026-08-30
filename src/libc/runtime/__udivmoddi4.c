/* libc/runtime/__udivmoddi4.c */

#include <stdint.h>
#include <stddef.h>

uint64_t __udivmoddi4(uint64_t numerator, uint64_t denominator, uint64_t* remainder) {
    uint64_t q = 0;
    uint64_t r = 0;
    int i;

    if (denominator == 0) {
        return 0;
    }

    for (i = 63; i >= 0; i--) {
        r <<= 1;
        r |= (numerator >> i) & 1;

        if (r >= denominator) {
            r -= denominator;
            q |= (uint64_t)1 << i;
        }
    }

    if (remainder != NULL) {
        *remainder = r;
    }

    return q;
}
