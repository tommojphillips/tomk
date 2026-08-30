/* libc/runtime/__udivdi3.c */

#include <stdint.h>
#include <stddef.h>

extern uint64_t __udivmoddi4(uint64_t numerator, uint64_t denominator, uint64_t* remainder);

uint64_t __udivdi3(uint64_t numerator, uint64_t denominator) {
    return __udivmoddi4(numerator, denominator, NULL);
}
