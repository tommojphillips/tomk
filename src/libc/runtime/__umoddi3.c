/* libc/runtime/__umoddi3.c */

#include <stdint.h>

extern uint64_t __udivmoddi4(uint64_t numerator, uint64_t denominator, uint64_t* remainder);

uint64_t __umoddi3(uint64_t numerator, uint64_t denominator) {
    uint64_t remainder = 0;
    __udivmoddi4(numerator, denominator, &remainder);
    return remainder;
}
