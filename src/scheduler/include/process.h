/* process.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _PROCESS_H
#define _PROCESS_H

#include <stdint.h>
#include <stddef.h>

#include <vmm.h>
#include <kheap.h>

/* PROC_FLAGS
 *
 *   -- Error condition (1 bit)
 *   |
 * | E X X X X X X X X X X X X X X X X X X X X X X X X X X X X M T T |
 *                                                             | |
 *                                                             | ----- Type (2 bits)
 *                                                             ------- Memory (1 bit)
 */
#define PROC_FLAG_NONE         0b00000000
#define PROC_FLAG_KERNEL       0b00000001 /* Process is executed in kernel mode */
#define PROC_FLAG_USER         0b00000011 /* Process is executed in user mode */
#define PROC_FLAG_MEMORY_STACK 0b00000100 /* Process has a memory stack */

#define PROC_FLAG_TYPE_MASK    0b00000011

#define PROC_FLAG_ERROR        0x80000000

typedef struct context_t {
    uint32_t cr3;
    uint32_t gs;
    uint32_t fs;
    uint32_t ds;
    uint32_t ss;
    uint32_t es;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} context_t;

typedef struct interrupt_frame_t {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} interrupt_frame_t;

typedef struct process_t process_t;

struct process_t {
    context_t context;
    interrupt_frame_t frame;
    size_t id;
    uint32_t flags;
    vmm_t vmm;
    heap_t heap;
    void* exe;
    size_t exe_size;
    size_t ticks; /* quantum */
    process_t* next;
    process_t* prev;
};

typedef struct procman_t {
    process_t* head;
    process_t* tail;
    process_t* current;
    size_t count;
} procman_t;

typedef void (*process_entry_fn_t)(void);

/* Create and add process to procman */
int process_create(procman_t* man, process_t** proc);

/* Remove and free process from procman */
void process_destroy(procman_t* man, process_t* proc);

/* Select the next process */
process_t* process_next(procman_t* man, process_t* proc);

/* Update process ticks, check if a process switch is needed */
int process_update(process_t* proc);

#endif
