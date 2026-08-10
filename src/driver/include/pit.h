/* driver/include/pit.h */

#ifndef DRIVER_PIT_H
#define DRIVER_PIT_H

#include <stdint.h>

extern volatile uint32_t timer_ticks;

void wait_ms(uint32_t ms);

#endif
