/* pic.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef DRIVER_PIC_H
#define DRIVER_PIC_H

#include <stdint.h>

/* Hardware reset PIC */
extern void pic_reset(void);

/* Disable all IRQs */
extern void pic_disable(void);

/* Send EOI event */
extern void pic_send_eoi(uint8_t irq);

/* Enable IRQ line */
extern void pic_enable_irq(uint8_t irq);

/* Disable IRQ line */
extern void pic_disable_irq(uint8_t irq);

/* Get IRR register */
extern uint16_t pic_get_irr(void);

/* Get ISR register */
extern uint16_t pic_get_isr(void);

#endif
