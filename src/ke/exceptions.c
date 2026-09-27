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
#include <stackframe.h>

#define vec_de      0x00 /* Fault - Division error */
#define vec_db      0x01 /* Trap  - Debug */
#define vec_bp      0x03 /* Trap  - Breakpoint */
#define vec_of      0x04 /* Trap  - Overflow */
#define vec_br      0x05 /* Fault - Bound range exceeded */
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

/* Integer division error */
#define INTEGER_DIVISION_ERROR          0x00000005

/* Code access violation */
#define CODE_ACCESS_VIOLATION           0x00000006

/* Unmapped code access */
#define UNMAPPED_CODE_ACCESS            0x00000007

/* NULL Dereference */
#define NULL_DEREFERENCE                0x00000008

static const char* error_names[] = {
    "Unknown error",
    "Unmapped write access",
    "Unmapped read access",
    "Write access violation",
    "Read access violation",
    "Integer divide by zero",
    "Code access violation",
    "Unmapped code access",
    "NULL dereference"
};

static void print_call_stack(exception_state_t* state) {
    stack_frame_t* frame = (stack_frame_t*)state->cpu.ebp;
    kprint("Call stack:\n 0x%08X\n", state->exception.eip);
    while (frame != NULL && pg_virt2phys((uintptr_t)frame) != 0) {
        kprint(" 0x%08X\n", frame->return_address - 1);
        frame = stackframe_next(frame);
    }
}

static void print_state(exception_state_t* state) {
    kprint("CS = 0x%8.4X\nEIP = 0x%8.8X\nF   = 0x%8.8X\n", state->exception.cs, state->exception.eip, state->exception.eflags);
    if (state->exception.cs & 0x3) {
        kprint("SS2 = 0x%8.4X\nESP2 = 0x%8.8X\n", state->exception.ss2, state->exception.esp2);
    }

    kprint("CR0 = 0x%02X\nCR2 = 0x%02X\nCR3 = 0x%02X\n" \
        "EAX = 0x%02X\nECX = 0x%02X\nEDX = 0x%02X\nEBX = 0x%02X\n" \
        "ESP = 0x%02X\nEBP = 0x%02X\nESI = 0x%02X\nEDI = 0x%02X\n" \
        "SS  = 0x%02X\nES  = 0x%02X\nDS  = 0x%02X\n" \
        "FS  = 0x%02X\nGS  = 0x%02X\n",
        state->cpu.cr0, state->cpu.cr2, state->cpu.cr3,
        state->cpu.eax, state->cpu.ecx, state->cpu.edx, state->cpu.ebx,
        state->cpu.esp, state->cpu.ebp, state->cpu.esi, state->cpu.edi,
        state->cpu.ss, state->cpu.es, state->cpu.ds, state->cpu.fs, state->cpu.gs);
}

static void de(process_t* proc, exception_state_t* state) {
    /* Division error */
    char buffer[32] = {0};
    const char* priv = (state->exception.cs & 0x3) ? "User" : "Kernel";
    size_t id = 0;

    if (proc != NULL) {
        id = proc->id;
    }

    i80386_mnem_get_str(state->exception.eip, buffer, 32);
    kprint("EXCEPTION:\n %s process (pid=%u)\n threw exception at 0x%08X\n %08X: Integer division by zero\n 0x%08X: %s\n",
        priv, id, state->exception.eip, INTEGER_DIVISION_ERROR, state->exception.eip, buffer);

    print_call_stack(state);
}
static void df(process_t* proc, exception_state_t* state) {
    /* Double fault */
    char buffer[32] = {0};
    const char* priv = (state->exception.cs & 0x3) ? "User" : "Kernel";
    size_t id = 0;

    if (proc != NULL) {
        id = proc->id;
    }

    i80386_mnem_get_str(state->exception.eip, buffer, 32);
    kprint("EXCEPTION:\n %s process (pid=%u)\n Threw exception at 0x%08X\n %08X: #DF(%X)\n 0x%08X: %s\n",
        priv, id, state->exception.eip, 0, state->exception.error, state->exception.eip, buffer);

    print_state(state);
    print_call_stack(state);
}
static void ts(process_t* proc, exception_state_t* state) {
    /* Task segment fault */
    char buffer[32] = {0};
    const char* priv = (state->exception.cs & 0x3) ? "User" : "Kernel";
    size_t id = 0;

    if (proc != NULL) {
        id = proc->id;
    }

    i80386_mnem_get_str(state->exception.eip, buffer, 32);
    kprint("EXCEPTION:\n %s process (pid=%u)\n Threw exception at 0x%08X\n %08X: #TS(%X)\n 0x%08X: %s\n",
        priv, id, state->exception.eip, 0, state->exception.error, state->exception.eip, buffer);

    print_state(state);
    print_call_stack(state);
}
static void np(process_t* proc, exception_state_t* state) {
    /* Not-Present fault */
    char buffer[32] = {0};
    const char* priv = (state->exception.cs & 0x3) ? "User" : "Kernel";
    size_t id = 0;

    if (proc != NULL) {
        id = proc->id;
    }

    i80386_mnem_get_str(state->exception.eip, buffer, 32);
    kprint("EXCEPTION:\n %s process (pid=%u)\n Threw exception at 0x%08X\n %08X: #NP(%X)\n 0x%08X: %s\n",
        priv, id, state->exception.eip, 0, state->exception.error, state->exception.eip, buffer);

    print_state(state);
    print_call_stack(state);
}
static void gp(process_t* proc, exception_state_t* state) {
    /* General protection fault */
    char buffer[32] = {0};
    const char* priv = (state->exception.cs & 0x3) ? "User" : "Kernel";
    size_t id = 0;

    if (proc != NULL) {
        id = proc->id;
    }

    i80386_mnem_get_str(state->exception.eip, buffer, 32);
    kprint("EXCEPTION:\n %s process (pid=%u)\n Threw exception at 0x%08X\n %08X: #GP(%X)\n 0x%08X: %s\n",
        priv, id, state->exception.eip, 0, state->exception.error, state->exception.eip, buffer);

    print_state(state);
    print_call_stack(state);
}
static void ud(process_t* proc, exception_state_t* state) {
    /* Undefined fault */
    char buffer[32] = {0};
    const char* priv = (state->exception.cs & 0x3) ? "User" : "Kernel";
    size_t id = 0;

    if (proc != NULL) {
        id = proc->id;
    }

    i80386_mnem_get_str(state->exception.eip, buffer, 32);
    kprint("EXCEPTION:\n %s process (pid=%u)\n Threw exception at 0x%08X\n %08X: #UD(%X)\n 0x%08X: %s\n",
        priv, id, state->exception.eip, 0, state->exception.error, state->exception.eip, buffer);

    print_state(state);
    print_call_stack(state);
}
static void ss(process_t* proc, exception_state_t* state) {
    /* Stack fault */
    char buffer[32] = {0};
    const char* priv = (state->exception.cs & 0x3) ? "User" : "Kernel";
    size_t id = 0;

    if (proc != NULL) {
        id = proc->id;
    }

    i80386_mnem_get_str(state->exception.eip, buffer, 32);
    kprint("EXCEPTION:\n %s process (pid=%u)\n Threw exception at 0x%08X\n %08X: #SS(%X)\n 0x%08X: %s\n",
        priv, id, state->exception.eip, 0, state->exception.error, state->exception.eip, buffer);

    print_state(state);
    print_call_stack(state);
}
static void pf(process_t* proc, exception_state_t* state) {
    /* Page fault */
    char buffer[32] = { 0 };
    const char* priv = (state->exception.cs & 0x3) ? "User" : "Kernel";
    int error = 0;
    size_t id = 0;

    if (proc != NULL) {
        id = proc->id;
    }

    i80386_mnem_get_str(state->exception.eip, buffer, 32);

    switch (state->exception.error & (PTE_P | PTE_RW)) {
        
        case (PTE_NP | PTE_RO):
            if (state->cpu.cr2 == state->exception.eip) {
                /* Treat unmapped addresses that have the RW bit clear and cr2 == eip as unmapped-code-access */
                error = UNMAPPED_CODE_ACCESS;
            }
            else if ((state->cpu.cr2 & 0xFFFFF000) == 0) {
                /* Treat ... */
                error = NULL_DEREFERENCE;
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
        
        case (PTE_NP | PTE_RW):
            if ((state->cpu.cr2 & 0xFFFFF000) == 0) {
                /* Treat ... */
                error = NULL_DEREFERENCE;
            }
            else {
                /* Treat unmapped addresses that have the RW bit set as a ummapped-write-access */
                error = UNMAPPED_WRITE_ACCESS;
            }
            break;

        /* Treat mapped addresses that have the RW bit set as a write-access-violation */
        case (PTE_P | PTE_RW):
            error = WRITE_ACCESS_VIOLATION;
            break;
    }
    
    kprint("EXCEPTION:\n %s process (pid=%u)\n Threw exception at 0x%08X: %s\n %08X: %s (%08X)\n",
        priv, id, state->exception.eip, buffer, error, error_names[error], state->cpu.cr2);

    print_call_stack(state);
}

void ex_fault_handler(uint8_t vector, exception_state_t* state) {
    process_t* proc = scheduler_current();
    
    switch (vector) {
        case vec_de:
            de(proc, state);
            break;
        case vec_df:
            df(proc, state);
            break;
        case vec_ts:
            ts(proc, state);
            break;
        case vec_np:
            np(proc, state);
            break;
        case vec_gp:
            gp(proc, state);
            break;
        case vec_ud:
            ud(proc, state);
            break;
        case vec_ss:
            ss(proc, state);
            break;
        case vec_pf:
            pf(proc, state);
            break;
    }
}
