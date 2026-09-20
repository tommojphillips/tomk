/* scheduler.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * Pre-emptive round robin scheduler
 */
 
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <assert.h>
#include <kdprint.h>
#include <i86.h>
#include <paging.h>
#include <vmm.h>
#include <kmalloc.h>
#include <kheap.h>
#include <kspacedef.h>
#include <scheduler.h>
#include <process.h>
#include <gdt.h>
#include <cswitch.h>

#define DEFAULT_STACK (1*1024*1024)

#define UVIRT     0x00001000
#define UVIRT_END KVIRT

/* Global scheduler instance */
static scheduler_t scheduler;

static process_t* create_proc(uintptr_t entry, uint32_t flags);
static void destroy_proc(process_t* proc, int ret);

static void idle_proc(void);
static void cleanup_proc(void);

void scheduler_init(void) {

    /* Create the kernel process */
    process_t* kinit = create_proc(0, PROC_FLAG_KERNEL);
    if (kinit == NULL) {
        kdprint("[SCHEDULER] Error failed to create kinit proc\n");
        return;
    }
    /* Set the current process to the (this) kernel process */
    scheduler.procman.current = kinit;
    
    /* Create the idle process */
    process_t* idle1 = create_proc((uintptr_t)idle_proc, PROC_FLAG_KERNEL);
    if (idle1 == NULL) {
        kdprint("[SCHEDULER] Error failed to create idle proc\n");
        return;
    }

    /* Setup PIT int handler */
    write_int_gate(0x20, 0x08, (uintptr_t)cswitch_handler);

	kprint("[SCHEDULER] Init OK\n");
}
process_t* scheduler_load_kprocess(uintptr_t entry) {
    /* Create process */
    return create_proc(entry, PROC_FLAG_KERNEL | PROC_FLAG_MEMORY_STACK);
}
process_t* scheduler_load_uprocess(uintptr_t entry) {
    /* Create process */
    return create_proc(entry, PROC_FLAG_USER | PROC_FLAG_MEMORY_STACK);
}
void scheduler_unload_kprocess(process_t* proc, int ret) {
    /* Destroy process */
    destroy_proc(proc, ret);
}
void scheduler_unload_uprocess(process_t* proc, int ret) {
    /* Destroy process */
    destroy_proc(proc, ret);
}

process_t* scheduler_current(void) {
    return scheduler.procman.current;
}
process_t* scheduler_head(void) {
    return scheduler.procman.head;
}
process_t* scheduler_tail(void) {
    return scheduler.procman.tail;
}

int scheduler_switch(process_t** current, process_t** next) {

    if (scheduler.procman.current == NULL) {
        scheduler.procman.current = scheduler.procman.head;
    }

    if (!process_update(scheduler.procman.current)) {
        *current = NULL;
        *next = NULL;
        return 0; /* no switch */
    }

    *current = scheduler.procman.current;
    *next = process_next(&scheduler.procman, scheduler.procman.current);
    scheduler.procman.current = *next;

    return *next != *current; /* switch? */
}

static process_t* create_proc(uintptr_t entry, uint32_t flags) {
    process_t* proc = NULL;
    uint16_t selcode = 0;
    uint16_t selstack = 0;
    uint16_t seldata = 0;

    switch (flags & PROC_FLAG_TYPE_MASK) {
        case PROC_FLAG_KERNEL:
            selcode = KCODE;
            selstack = KSTACK;
            seldata = KDATA;
            break;
        case PROC_FLAG_USER:
            selcode = UCODE | 3;
            selstack = USTACK | 3;
            seldata = UDATA | 3;
            break;
    }

    /* Create process */
    if (!process_create(&scheduler.procman, &proc)) {
        return NULL;
    }
    
    /* Init process */

    /* Set general registers */
    proc->context.eax = 0;
    proc->context.edx = 0;
    proc->context.ecx = 0;
    proc->context.ebx = 0;
    proc->context.ebp = 0;
    proc->context.esi = 0;
    proc->context.edi = 0;
    
    /* Set segment registers */
    proc->context.es = seldata;
    proc->context.ds = seldata;
    proc->context.fs = seldata;
    proc->context.gs = seldata;

    /* Setup stack */
    void* stack = kmalloc(DEFAULT_STACK);
    if (stack == NULL) {
        return NULL;
    }

    /* Set the stack to the top of stack */
    stack += DEFAULT_STACK;

    /* Inject cleanup return function */
    stack -= sizeof(uintptr_t);
    *((uintptr_t*)stack) = (uintptr_t)&cleanup_proc;
    
    /* Set stack pointer */
    proc->context.esp = (uintptr_t)stack;
    proc->context.ss = selstack;

    /* Setup interrupt frame */
    proc->frame.eip = (uint32_t)entry;
    proc->frame.cs = selcode;
    proc->frame.eflags = 0x202;

    proc->id = scheduler.procman.count++;
    proc->flags = flags;

    /* Create page directory, Map kernel space into the processes address space */
    proc->context.cr3 = pgd_create_pd();
    
    /* Init memory stack ( VMM -> HEAP ) */
    if (flags & PROC_FLAG_MEMORY_STACK) {
        vmm_init(&proc->vmm, UVIRT, UVIRT_END, kmalloc);
        heap_init(&proc->heap, &proc->vmm);
    }

	kdprint("[SCHEDULER] Process created pid=%u flags=%u\n", proc->id, flags);
    return proc;
}

static void destroy_proc(process_t* proc, int ret) {

    if (proc == NULL) {
        return;
    }

    kdprint("[SCHEDULER] Process destroyed pid=%u flags=%u\n", proc->id, proc->flags);

    if (proc->flags & PROC_FLAG_MEMORY_STACK) {
        /* Destroy memory stack ( VMM -> HEAP ) */
        heap_destroy(&proc->heap);
        vmm_destroy(&proc->vmm, kfree);
    }

    /* Destroy page directory */
    pgd_destroy_pd(proc->context.cr3);

    if (proc->exe != NULL) {
        kfree(proc->exe);
        proc->exe = NULL;
        proc->exe_size = 0;
    }
    
    /* Destroy process */
    process_destroy(&scheduler.procman, proc);
    
    if (scheduler.procman.current == proc) {
        /* We are unloading ourself, spin untill we die */
        scheduler.procman.current = NULL;
        while (1) {
            haltwait();
        }
    }
}

/* Process cleanup */
static void cleanup_proc(void) {
    process_t* proc = scheduler_current();
    kdprint("[SCHEDULER] Process cleanup pid=%u flags=%u\n", proc->id, proc->flags);
    scheduler_unload_kprocess(proc, 0);
}

/* Idle process */
static void idle_proc(void) {
    while (1) { 
        haltwait();
    }
}
