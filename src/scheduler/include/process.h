/* process.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * Process 
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

#define PROC_FLAG_NONE         0    /* Process has no flags */
#define PROC_FLAG_KERNEL       0x01 /* Process is executed in kernel mode */
#define PROC_FLAG_USER         0x03 /* Process is executed in user mode */
#define PROC_FLAG_STACK        0x04 /* Process has a stack */
#define PROC_FLAG_HEAP         0x08 /* Process has a heap */
#define PROC_SHARED_ADDR_SPACE 0x10 /* Process shares kernel address space */

#define PROC_FLAG_TYPE_MASK    0x03

#define PROC_FLAG_CREATE       0x20000000
#define PROC_FLAG_KILL         0x40000000
#define PROC_FLAG_ERROR        0x80000000
#define PROC_FLAG_SERVICE_MASK 0xC0000000

/* Process context */
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

/* Interrupt frame */
typedef struct interrupt_frame_t {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} interrupt_frame_t;

/* Process entry point */
typedef void (*process_entry_fn_t)(void);

/* Process */
typedef struct process_t process_t;

/* Process */
struct process_t {
    context_t context;
    interrupt_frame_t frame;
    size_t id;
    process_entry_fn_t entry;
    uint32_t flags;
    const char* path;
    vmm_t vmm;
    heap_t heap;
    void* exe;
    size_t exe_size;
    void* stack;
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

/* Create process 
 proc: The process to create 
 Returns: non-zero if success, otherwise 0. */
int process_create(process_t** proc);

/* Destroy process 
 proc: The process to destroy */
void process_destroy(process_t* proc);

/* Update process
 proc: The process to update
 Returns: non-zero if process is done, otherwise returns 0 */
int process_update(process_t* proc);

#endif
