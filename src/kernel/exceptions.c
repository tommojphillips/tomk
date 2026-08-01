/* exceptions.c - exception/interrupt handlers */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <i86.h>

void print_cpu_state(CPU_STATE* state) {
    printf("LA  = 0x%X:0x%X\nF   = 0x%X\nCR0 = 0x%X\nCR2 = 0x%X\nCR3 = 0x%X\nEAX = 0x%X\nECX = 0x%X\nEDX = 0x%X\nEBX = 0x%X\n" \
        "ESP = 0x%X\nEBP = 0x%X\nESI = 0x%X\nEDI = 0x%X\n" \
        "CS  = 0x%X\nSS  = 0x%X\nES  = 0x%X\nDS  = 0x%X\nFS  = 0x%X\nGS  = 0x%X\n", 
        state->int_cs, state->int_eip, state->int_eflags, state->cr0, state->cr2, state->cr3,
        state->eax, state->ecx, state->edx, state->ebx,
        state->esp, state->ebp, state->esi, state->edi,
        state->cs, state->ss, state->es, state->ds, state->fs, state->gs);
}
void exception_dbz(CPU_STATE* state) {
    /* DBZ - ITC 0 */
    printf("\n#DBZ:\n");
    print_cpu_state(state);
}
void exception_trap(CPU_STATE* state) {
    /* TRAP - ITC 1 */
    printf("\n#TRAP:\n");
    print_cpu_state(state);
}
void exception_nmi(CPU_STATE* state) {
    /* NMI - ITC 2 */
    printf("\n#NMI:\n");
    print_cpu_state(state);
}
void exception_int3(CPU_STATE* state) {
    /* INT3 - ITC 3 */
    printf("\n#INT3:\n");
    print_cpu_state(state);
}
void exception_of(CPU_STATE* state) {
    /* OF - ITC 4 */
    printf("\n#OF:\n");
    print_cpu_state(state);
}
void exception_bound(CPU_STATE* state) {
    /* BOUND - ITC5 */
    printf("\n#BOUND:\n");
    print_cpu_state(state);
}
void exception_ud(CPU_STATE* state) {
    /* Undefined fault */
    printf("\n#UD:\n");
    print_cpu_state(state);
}
void exception_df(CPU_STATE* state) {
    /* Double fault */
    printf("\n#DF(%X):\n", state->int_error);
    print_cpu_state(state);
}
void exception_ts(CPU_STATE* state) {
    /* Task segment fault */
    printf("\n#TS(%X):\n", state->int_error);
    print_cpu_state(state);
}
void exception_np(CPU_STATE* state) {
    /* Not present fault */
    printf("\n#NP(%X):\n", state->int_error);
    print_cpu_state(state);
}
void exception_ss(CPU_STATE* state) {
    /* Stack segment fault */
    printf("\n#SS(%X):\n", state->int_error);
    print_cpu_state(state);
}
void exception_gp(CPU_STATE* state) {
    /* General protection fault */
    printf("\n#GP(%X):\n", state->int_error);
    print_cpu_state(state);
}
void exception_pf(CPU_STATE* state) {
    /* Page fault */
    printf("\n#PF(%X):\n", state->int_error);
    print_cpu_state(state);
}
