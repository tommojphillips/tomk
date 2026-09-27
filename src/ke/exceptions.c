/* exceptions.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <kdprint.h>
#include <kpanic.h>

#include <i86.h>
#include <paging.h>
#include <i80386_mnem.h>
#include <scheduler.h>
#include <process.h>

#define vec_dbz     0x00 /* Fault - Division error */
#define vec_trap    0x01 /* Trap  - Debug */
#define vec_nmi     0x02 /* Non-maskable interrupt */
#define vec_int3    0x03 /* Trap  - Breakpoint */
#define vec_of      0x04 /* Trap  - Overflow */
#define vec_bound   0x05 /* Fault - Bound range exceeded */
#define vec_ud      0x06 /* Fault - Invalid opcode */
#define vec_df      0x08 /* Abort - Double fault */
#define vec_ts      0x0A /* Fault - Invalid TSS */
#define vec_np      0x0B /* Fault - Segment not present */
#define vec_ss      0x0C /* Fault - stack segment fault */
#define vec_gp      0x0D /* Fault - General protection fault */
#define vec_pf      0x0E /* Fault - Page fault */

/* Unmapped write access */
#define UNMAPPED_WRITE_ACCESS           0x00000001

/* Unmapped read access */
#define UNMAPPED_READ_ACCESS            0x00000002

/* Write access violation */
#define WRITE_ACCESS_VIOLATION          0x00000003

/* Read access violation */
#define READ_ACCESS_VIOLATION           0x00000004

/* Divide by zero */
#define INTEGER_DIVIDE_BY_ZERO          0x00000005

/* Code access violation */
#define CODE_ACCESS_VIOLATION           0x00000006

/* Unmapped code access */
#define UNMAPPED_CODE_ACCESS            0x00000007

static const char* exception_names[] = {
    "DBZ",
    "TRAP",
    "NMI",
    "INT3",
    "OF",
    "BOUND",
    "UD",
    "DF",
    "TS",
    "NP",
    "SS",
    "GP",
    "PF"
};

static const char* error_names[] = {
    "Unknown error",
    "Unmapped write access",
    "Unmapped read access",
    "Write access violation",
    "Read access violation",
    "Integer divide by zero",
    "Code access violation"
};

static void print_state(exception_state_t* state) {
    kprint("F   = 0x%02X\nCR0 = 0x%02X\nCR2 = 0x%02X\nCR3 = 0x%02X\nEAX = 0x%02X\nECX = 0x%02X\nEDX = 0x%02X\nEBX = 0x%02X\n" \
        "ESP = 0x%02X\nEBP = 0x%02X\nESI = 0x%02X\nEDI = 0x%02X\n" \
        "CS  = 0x%02X\nSS  = 0x%02X\nES  = 0x%02X\nDS  = 0x%02X\nFS  = 0x%02X\nGS  = 0x%02X\n", 
        state->exception.eflags, state->cpu.cr0, state->cpu.cr2, state->cpu.cr3,
        state->cpu.eax, state->cpu.ecx, state->cpu.edx, state->cpu.ebx,
        state->cpu.esp, state->cpu.ebp, state->cpu.esi, state->cpu.edi,
        state->cpu.cs, state->cpu.ss, state->cpu.es, state->cpu.ds, state->cpu.fs, state->cpu.gs);
}

static void dbz(process_t* proc, exception_state_t* state) {
    /* DBZ */
    char buffer[32] = {0};
    const char* priv = (state->exception.cs & 0x3) ? "User" : "Kernel";

    i80386_mnem_get_str(state->exception.eip, buffer, 32);
    kprint("EXCEPTION:\n %s process (pid=%u)\n threw exception at 0x%08X\n %08X: Integer division by zero\n 0x%08X: %s\n",
        priv, proc->id, state->exception.eip, INTEGER_DIVIDE_BY_ZERO, state->exception.eip, buffer);
}
static void gp(process_t* proc, exception_state_t* state, const char* name) {
    /* Fault */
    char buffer[32] = {0};
    const char* priv = (state->exception.cs & 0x3) ? "User" : "Kernel";

    i80386_mnem_get_str(state->exception.eip, buffer, 32);
    kprint("EXCEPTION:\n %s process (pid=%u)\n Threw exception at 0x%08X\n %08X: #%s(%X)\n 0x%08X: %s\n",
        priv, proc->id, state->exception.eip, 0, name, state->exception.error, state->exception.eip, buffer);

    print_state(state);
}
static void pf(process_t* proc, exception_state_t* state) {
    /* Page fault */
    char buffer[32] = { 0 };
    const char* priv = (state->exception.cs & 0x3) ? "User" : "Kernel";
    int error = 0;

    i80386_mnem_get_str(state->exception.eip, buffer, 32);

    switch (state->exception.error & (PTE_P | PTE_RW)) {
        
        case (PTE_NP | PTE_RO):
            if (state->cpu.cr2 == state->exception.eip) {
                /* Treat unmapped addresses that have the RW bit clear and cr2 == eip as unmapped-code-access */
                error = UNMAPPED_CODE_ACCESS;
            }
            else {
            /* Treat unmapped addresses that have the RW bit clear as a ummapped-read-access */
                error = UNMAPPED_READ_ACCESS;
            }
            break;

        case (PTE_P | PTE_RO):
            if (state->cpu.cr2 == state->exception.eip) {
                /* Treat mapped addresses that have the RW bit clear and cr2 == eip as code-access-violation */
                error = CODE_ACCESS_VIOLATION;
            }
            else {
                /* Treat mapped addresses that have the RW bit clear as a read-access-violation */
                error = READ_ACCESS_VIOLATION;
            }
            break;
        
        /* Treat unmapped addresses that have the RW bit set as a ummapped-write-access */        
        case (PTE_NP | PTE_RW):
            error = UNMAPPED_WRITE_ACCESS;
            break;

        /* Treat mapped addresses that have the RW bit set as a write-access-violation */
        case (PTE_P | PTE_RW):
            error = WRITE_ACCESS_VIOLATION;
            break;
    }
    
    kprint("EXCEPTION:\n %s process (pid=%u)\n Threw exception at 0x%08X: %s\n %08X: %s (%08X)\n",
        priv, proc->id, state->exception.eip, buffer, error, error_names[error], state->cpu.cr2);
}

void ex_fault_handler(uint8_t vector, exception_state_t* state) {
    process_t* proc = scheduler_current();
    
    switch (vector) {
        case vec_dbz:
            dbz(proc, state);
            break;

        case vec_ud:
        case vec_df:
        case vec_ts:
        case vec_np:
        case vec_ss:
        case vec_gp:
            gp(proc, state, exception_names[vector]);
            break;

        case vec_pf:
            pf(proc, state);
            break;
    }
}
