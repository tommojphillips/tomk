/* i86.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef I86_H
#define I86_H

#include <stdint.h>

typedef struct exception_frame_t {
    uint32_t error;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp2;
    uint32_t ss2;
} exception_frame_t;

typedef struct cpu_state_t {
    uint32_t cr0;
    uint32_t cr2;
    uint32_t cr3;

    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;

    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
} cpu_state_t;

typedef struct exception_state_t {
    cpu_state_t cpu;
    exception_frame_t exception;
} exception_state_t;

/* Input 8bit value from IO port
 port:   the IO port
 Returns 8bit value read from IO port  */
extern uint8_t inb(uint16_t port);

/* Input 16bit value from IO port/s
 port:   the IO port
 Returns 16bit value read from IO port/s */
extern uint16_t inw(uint16_t port);

/* Input 32bit value from IO port/s
 port:   the IO port
 Returns 32bit value read from IO port/s */
extern uint32_t ind(uint16_t port);

/* Output 8bit value to IO port
 port:  the IO port
 value: the 8bit value to write to IO port */
extern void outb(uint16_t port, uint8_t value);

/* Output 16bit value to IO port/s
 port:  the IO port
 value: the 16bit value to write to IO port/s */
extern void outw(uint16_t port, uint16_t value);

/* Output 32bit value to IO port/s
 port:  the IO port
 value: the 32bit value to write to IO port/s */
extern void outd(uint16_t port, uint32_t value);

/* Get ESP
 Returns ESP */
extern uint32_t getesp(void);

/* Get CR0
 Returns CR0 */
extern uint32_t getcr0(void);

/* Set CR0
 value: the value to set cr0
 Returns CR0 after assignment */
extern uint32_t setcr0(uint32_t value);

/* Get CR2
 Returns CR2 */
extern uint32_t getcr2(void);

/* Set CR2
 value: the value to set cr2
 Returns CR2 after assignment */
extern uint32_t setcr2(uint32_t value);

/* Get CR3
 Returns CR3 */
extern uint32_t getcr3(void);

/* Set CR3
 value: the value to set cr3
 Returns CR3 after assignment */
extern uint32_t setcr3(uint32_t value);

/* Spin Wait CPU x amount of times
 spins: */
extern void spinwait(uint32_t spins);

/* Wait using hlt instruction */
extern void haltwait(void);

/* Halt CPU */
extern void halt(void);

/* Write INT Gate to IDT
 vector:   IDT index
 selector: int CS
 offset:   int EIP */
extern void write_int_gate(uint8_t vector, uint16_t selector, uint32_t offset);

/* Write TASK Gate to IDT
 vector:   IDT index
 selector: int CS
 offset:   int EIP */
extern void write_task_gate(uint8_t vector, uint16_t selector);

/* Write TSS Gate to GDT
 selector: GDT index
 limit:    segment limit
 ar:       tss ar word
 base:     segment base */
extern void write_tss_descriptor(uint16_t selector, uint32_t limit, uint16_t ar, uint32_t base);

#endif
