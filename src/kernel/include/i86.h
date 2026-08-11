/* i86.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef I86_H
#define I86_H

#include <stdint.h>

#define TSS_SIZE 0x1000
#define IDT_SIZE 0x1000
#define GDT_SIZE 0x1000

typedef struct cpu_regs_t {
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
} cpu_regs_t;

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

    uint32_t int_error;
    uint32_t int_eip;
    uint32_t int_cs;
    uint32_t int_eflags;
} cpu_state_t;

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

/* Invoke Interrupt 
 vector:       interrupt to invoke
 input_regs:   the gpr values to invoke the interrupt with 
 output_state: the cpu state after the interrupt has been invoked */
extern void int86(uint8_t vector, const cpu_regs_t* input_regs, cpu_state_t* output_state);

/* Set GPRs 
 regs: the GPRs to set */
extern void setregs(cpu_regs_t* regs);

/* Get GPRs 
 regs: the GPRs */
extern void getregs(cpu_regs_t* regs);

/* Get CPU State
 state: the cpu state */
extern void getstate(const cpu_state_t* state);

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

/* Get DR0
 Returns DR0 */
extern uint32_t getdr0(void);

/* Set DR0
 value: the value to set dr0
 Returns DR0 after assignment */
extern uint32_t setdr0(uint32_t value);

/* Get DR1
 Returns DR1 */
extern uint32_t getdr1(void);

/* Set DR1
 value: the value to set dr1
 Returns DR1 after assignment */
extern uint32_t setdr1(uint32_t value);

/* Get DR2
 Returns DR2 */
extern uint32_t getdr2(void);

/* Set DR2
 value: the value to set dr2
 Returns DR2 after assignment */
extern uint32_t setdr2(uint32_t value);

/* Get DR3
 Returns DR3 */
extern uint32_t getdr3(void);

/* Set DR3
 value: the value to set dr3
 Returns DR3 after assignment */
extern uint32_t setdr3(uint32_t value);

/* Get DR6
 Returns DR6 */
extern uint32_t getdr6(void);

/* Set DR6
 value: the value to set dr6
 Returns DR6 after assignment */
extern uint32_t setdr6(uint32_t value);

/* Get DR7
 Returns DR7 */
extern uint32_t getdr7(void);

/* Set DR7
 value: the value to set dr7
 Returns DR7 after assignment */
extern uint32_t setdr7(uint32_t value);

/* Enable CPU Interrupts */
extern void enable_interrupts(void);

/* Disable CPU Interrupts */
extern void disable_interrupts(void);

/* Spin Wait CPU x amount of times
 spins: */
extern void spinwait(uint32_t spins);

/* Wait using hlt instruction. */
extern void haltwait(void);

/* Write INT Gate to IDT
 vector:   IDT index
 ar:       int ar byte 
 selector: int CS
 offset:   int EIP */
extern void write_int_gate(uint8_t vector, uint8_t ar, uint16_t selector, uint32_t offset);

/* Write TSS Gate to GDT
 selector: GDT index
 ar:       tss ar byte
 limit:    segment limit
 base:     segment base */
extern void write_tss_gate(uint16_t selector, uint8_t ar, uint32_t limit, uint32_t base);

#endif
