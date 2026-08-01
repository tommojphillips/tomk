/* driver/include/pic.h */

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

#endif