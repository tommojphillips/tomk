/* scheduler.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
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

static process_t* create_kprocess(uintptr_t entry, uint8_t* data, size_t size);
static void idle_proc1(void);
static void cleanup_proc(void);

void scheduler_init(void) {

    /* Create the kernel process */
    process_t* kinit = create_kprocess(0, NULL, 0);
    if (kinit == NULL) {
        kdprint("[SCHEDULER] Error failed to create kinit proc\n");
        return;
    }
    /* Set the current process to the (this) kernel process */
    scheduler.procman.current = kinit;
    
    /* Create the idle process */
    process_t* idle1 = create_kprocess((uintptr_t)idle_proc1, NULL, 0);
    if (idle1 == NULL) {
        kdprint("[SCHEDULER] Error failed to create idle1 proc\n");
        return;
    }

    /* Setup PIT int handler */
    write_int_gate(0x20, 0x08, (uintptr_t)cswitch_handler);

	kprint("[SCHEDULER] Init OK\n");
}
process_t* scheduler_load_kprocess(uintptr_t entry) {       
    /* Create process */
    return create_kprocess(entry, NULL, 0);
}
void scheduler_unload_kprocess(process_t* proc) {

    if (proc == NULL) {
        return;
    }

    /* Destroy memory stack ( VMM -> HEAP ) */
    heap_destroy(&proc->heap);
    vmm_destroy(&proc->vmm, kfree);

    /* Destroy page directory */
    pgd_destroy_pd(proc->context.cr3);

    if (proc->exe != NULL) {
        kfree(proc->exe);
        proc->exe = NULL;
        proc->exe_size = 0;
    }
    
    kdprint("[SCHEDULER] Process destroyed id=%u\n", proc->id);

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
process_t* scheduler_get_current_process(void) {
    return scheduler.procman.current;
}
process_t* scheduler_get_head(void) {
    return scheduler.procman.head;
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

static process_t* create_kprocess(uintptr_t entry, uint8_t* data, size_t size) {
    process_t* proc = NULL;

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
    proc->context.es = KDATA;
    proc->context.ds = KDATA;
    proc->context.fs = KDATA;
    proc->context.gs = KDATA;

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
    proc->context.ss = KSTACK;

    /* Setup interrupt frame */
    proc->frame.eip = (uint32_t)entry;
    proc->frame.cs = KCODE;
    proc->frame.eflags = 0x202;

    proc->exe = data;
    proc->exe_size = size;
    proc->id = scheduler.procman.count++;

    /* Create page directory, Map kernel space into the processes address space */
    proc->context.cr3 = pgd_create_pd();
    
    /* Init memory stack ( VMM -> HEAP ) */
    vmm_init(&proc->vmm, UVIRT, UVIRT_END, kmalloc);
    heap_init(&proc->heap, &proc->vmm);

	kdprint("[SCHEDULER] kprocess created id=%u eip=0x%08X\n", proc->id, proc->frame.eip);
    return proc;
}

/* Process cleanup */
static void cleanup_proc(void) {
    process_t* proc = scheduler_get_current_process();
    kdprint("[SCHEDULER] kprocess cleanup id=%u eip=0x%08X\n", proc->id, proc->frame.eip);
    scheduler_unload_kprocess(proc);
}

/* Idle process */
static void idle_proc1(void) {
    while (1) { 
        haltwait();
    }
}
