/* libc/runtime/__udivmoddi4.c */

#include <stdint.h>
#include <stddef.h>

uint64_t __udivmoddi4(uint64_t num, uint64_t den, uint64_t* rem) {
    uint64_t quotient = 0;
    uint64_t remainder = 0;
    int i;

    if (den == 0) {
        return 0;
    }

    for (i = 63; i >= 0; i--) {
        remainder <<= 1;
        remainder |= (num >> i) & 1;

        if (remainder >= den) {
            remainder -= den;
            quotient |= (uint64_t)1 << i;
        }
    }

    if (rem != NULL) {
        *rem = remainder;
    }

    return quotient;
}
