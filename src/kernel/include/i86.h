/* kernel/include/i86.h */

#ifndef I86_H
#define I86_H

#include <stdint.h>

typedef struct CPU_REGS {
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
} CPU_REGS;

typedef struct CPU_STATE {
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
} CPU_STATE;

extern uint8_t inb(uint16_t port);
extern uint16_t inw(uint16_t port);
extern uint32_t ind(uint16_t port);

extern void outb(uint16_t port, uint8_t value);
extern void outw(uint16_t port, uint16_t value);
extern void outd(uint16_t port, uint32_t value);

extern void int86(uint8_t vector, const CPU_REGS* input_regs, CPU_STATE* output_state);
extern void setregs(CPU_REGS* regs);
extern void getregs(CPU_REGS* regs);

/* Get CPU State
 Returns pointer to CPU State */
extern void getstate(const CPU_STATE* state);

/* Get CR0
 Returns CR0 */
extern uint32_t getcr0();

/* Set CR0
 Returns CR0 after assignment */
extern uint32_t setcr0(uint32_t value);

/* Get CR2
 Returns CR2 */
extern uint32_t getcr2();

/* Set CR2
 Returns CR2 after assignment */
extern uint32_t setcr2(uint32_t value);

/* Get CR3
 Returns CR3 */
extern uint32_t getcr3();

/* Set CR3
 Returns CR3 after assignment */
extern uint32_t setcr3(uint32_t value);

#endif
