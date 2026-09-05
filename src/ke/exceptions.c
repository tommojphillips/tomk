/* exceptions.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <i86.h>
#include <paging.h>
#include <i80386_mnem.h>

#include <kdprint.h>
#include <kernel.h>

/* Unmapped write access */
#define UNMAPPED_WRITE_ACCESS           0xA0000001

/* Unmapped read access */
#define UNMAPPED_READ_ACCESS            0xA0000002

/* Write access violation */
#define WRITE_ACCESS_VIOLATION          0xA0000003

/* Read access violation */
#define READ_ACCESS_VIOLATION           0xA0000004

/* Divide by zero */
#define INTEGER_DIVIDE_BY_ZERO          0xA0000005

void print_cpu_state(cpu_state_t* state) {
    char buffer[32] = {0};
    i80386_mnem_get_str(state->int_eip, buffer, 32);
    kprint(" at\n0x%02X: %s\n", state->int_eip, buffer);
    kprint("\nF   = 0x%02X\nCR0 = 0x%02X\nCR2 = 0x%02X\nCR3 = 0x%02X\nEAX = 0x%02X\nECX = 0x%02X\nEDX = 0x%02X\nEBX = 0x%02X\n" \
        "ESP = 0x%02X\nEBP = 0x%02X\nESI = 0x%02X\nEDI = 0x%02X\n" \
        "CS  = 0x%02X\nSS  = 0x%02X\nES  = 0x%02X\nDS  = 0x%02X\nFS  = 0x%02X\nGS  = 0x%02X\n", 
        state->int_eflags, state->cr0, state->cr2, state->cr3,
        state->eax, state->ecx, state->edx, state->ebx,
        state->esp, state->ebp, state->esi, state->edi,
        state->cs, state->ss, state->es, state->ds, state->fs, state->gs);
}
void exception_dbz(cpu_state_t* state) {
    /* DBZ - ITC 0 */
    char buffer[32] = {0};
    const char* priv = "Super";
    if (state->int_error & PTE_US) {
        priv = "User";
    }
    i80386_mnem_get_str(state->int_eip, buffer, 32);
    kpanic(" Exception thrown at 0x%08X\n %08X: Integer division by zero (%s)\n  -> %s\n",
        state->int_eip, INTEGER_DIVIDE_BY_ZERO, priv, buffer);
}
void exception_trap(cpu_state_t* state) {
    /* TRAP - ITC 1 */
    kprint("\n#TRAP");
    print_cpu_state(state);
}
void exception_nmi(cpu_state_t* state) {
    /* NMI - ITC 2 */
    kprint("\n#NMI");
    print_cpu_state(state);
}
void exception_int3(cpu_state_t* state) {
    /* INT3 - ITC 3 */
    kprint("\n#INT3");
    print_cpu_state(state);
}
void exception_of(cpu_state_t* state) {
    /* OF - ITC 4 */
    kprint("\n#OF");
    print_cpu_state(state);
}
void exception_bound(cpu_state_t* state) {
    /* BOUND - ITC5 */
    kprint("\n#BOUND");
    print_cpu_state(state);
}
void exception_ud(cpu_state_t* state) {
    /* Undefined fault */
    kprint("\n#UD");
    print_cpu_state(state);
}
void exception_df(cpu_state_t* state) {
    /* Double fault */
    kprint("\n#DF(%X)", state->int_error);
    print_cpu_state(state);
}
void exception_ts(cpu_state_t* state) {
    /* Task segment fault */
    kprint("\n#TS(%X)", state->int_error);
    print_cpu_state(state);
}
void exception_np(cpu_state_t* state) {
    /* Not present fault */
    kprint("\n#NP(%X)", state->int_error);
    print_cpu_state(state);
}
void exception_ss(cpu_state_t* state) {
    /* Stack segment fault */
    kprint("\n#SS(%X)", state->int_error);
    print_cpu_state(state);
}
void exception_gp(cpu_state_t* state) {
    /* General protection fault */
    kprint("\n#GP(%X)", state->int_error);
    print_cpu_state(state);
}
void exception_pf(cpu_state_t* state) {
    /* Page fault */
    char buffer[32] = { 0 };
    const char* priv = "Super";

    if (state->int_error & PTE_US) {
        priv = "User";
    }

    switch (state->int_error & (PTE_P | PTE_RW)) {
        
        /* Treat unmapped addresses that have the RW bit clear as a ummapped-read-access */
        case (PTE_NP | PTE_RO):
            i80386_mnem_get_str(state->int_eip, buffer, 32);
            kpanic(" Exception thrown at 0x%08X\n %08X: Unmapped read access (%s)\n reading location 0x%08X -> %s\n",
                state->int_eip, UNMAPPED_READ_ACCESS, priv, state->cr2, buffer);
            break;

        /* Treat mapped addresses that have the RW bit clear as a read-access-violation */
        case (PTE_P | PTE_RO):
            i80386_mnem_get_str(state->int_eip, buffer, 32);
            kpanic(" Exception thrown at 0x%08X\n %08X: Read access violation (%s)\n reading location 0x%08X -> %s\n",
                state->int_eip, READ_ACCESS_VIOLATION, priv, state->cr2, buffer);
            break;
        
        /* Treat unmapped addresses that have the RW bit set as a ummapped-write-access */        
        case (PTE_NP | PTE_RW):
            i80386_mnem_get_str(state->int_eip, buffer, 32);
            kpanic(" Exception thrown at 0x%08X\n %08X: Unmapped write access (%s)\n writing location 0x%08X -> %s\n",
                state->int_eip, UNMAPPED_WRITE_ACCESS, priv, state->cr2, buffer);
            break;

        /* Treat mapped addresses that have the RW bit set as a write-access-violation */
        case (PTE_P | PTE_RW):
            i80386_mnem_get_str(state->int_eip, buffer, 32);
            kpanic(" Exception thrown at 0x%08X\n %08X: Write access violation (%s)\n writing location 0x%08X -> %s\n",
                state->int_eip, WRITE_ACCESS_VIOLATION, priv, state->cr2, buffer);
            break;
    }    
}
