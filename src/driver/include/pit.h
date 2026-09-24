/* pit.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef DRIVER_PIT_H
#define DRIVER_PIT_H

#include <stdint.h>

typedef void (*pit_handler_fn_t)(void);

/* PIT set timer int handler */
void pit_set_handler(pit_handler_fn_t handler);

/* PIT enable timer */
void pit_enable(void);

/* PIT disable timer */
void pit_disable(void);

/* PIT set frequency in hz */
void pit_set_freq(uint16_t interval_hz);

#endif
