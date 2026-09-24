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
#include <pit.h>
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

static process_t* create_proc(process_entry_fn_t entry, uint32_t flags);
static void destroy_proc(process_t* proc, int wait_self_destroy);

static void idle_proc(void);
static void cleanup_proc(void);

void scheduler_init(void) {

    /* Create the kernel process */
    process_t* kinit = create_proc(NULL, PROC_FLAG_KERNEL);
    if (kinit == NULL) {
        return;
    }

    /* Set the current process to the (this) kernel process */
    scheduler.current = kinit;
    
    /* Create the idle process */
    process_t* idle1 = create_proc(idle_proc, PROC_FLAG_KERNEL | PROC_FLAG_STACK);
    if (idle1 == NULL) {
        return;
    }

    /* Setup timer */
    pit_set_handler(cswitch_handler);
    pit_set_freq(1000); /* 50Hz */
    pit_enable();

	kprint("[SCHEDULER] Init OK\n");
}
process_t* scheduler_load_kprocess(process_entry_fn_t entry) {
    /* Create process */
    return create_proc(entry, PROC_FLAG_KERNEL | PROC_FLAG_STACK | PROC_FLAG_HEAP);
}
process_t* scheduler_load_uprocess(process_entry_fn_t entry) {
    /* Create process */
    return create_proc(entry, PROC_FLAG_USER | PROC_FLAG_STACK | PROC_FLAG_HEAP);
}
void scheduler_unload_kprocess(process_t* proc, int wait_self_destroy) {
    /* Destroy process */
    destroy_proc(proc, wait_self_destroy);
}
void scheduler_unload_uprocess(process_t* proc, int wait_self_destroy) {
    /* Destroy process */
    destroy_proc(proc, wait_self_destroy);
}

process_t* scheduler_current(void) {
    return scheduler.current;
}
process_t* scheduler_head(void) {
    return scheduler.head;
}
process_t* scheduler_tail(void) {
    return scheduler.tail;
}

int scheduler_switch(process_t** current, process_t** next) {

    if (scheduler.current == NULL) {
        scheduler.current = scheduler.head;
    }

    if (!process_update(scheduler.current)) {
        *current = NULL;
        *next = NULL;
        return 0; /* no switch */
    }

    *current = scheduler.current;
    
    if (scheduler.current->next != NULL) {
        *next = scheduler.current->next;
    }
    else {
        *next = scheduler.head;
    }

    scheduler.current = *next;

    return *next != *current; /* switch? */
}

static process_t* create_proc(process_entry_fn_t entry, uint32_t flags) {
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
    if (!process_create(&proc)) {
        kdprint("[SCHEDULER] Error failed to create process\n");
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
    proc->context.ss = selstack;
    
    if (flags & PROC_FLAG_STACK) {
        /* Setup stack */
        proc->stack = kmalloc(DEFAULT_STACK);
        if (proc->stack == NULL) {
            kdprint("[SCHEDULER] Error failed allocate process stack\n");
            return NULL;
        }

        /* Inject cleanup return function */
        void* stack = proc->stack + DEFAULT_STACK - sizeof(uintptr_t);
        *((uintptr_t*)stack) = (uintptr_t)&cleanup_proc;
    
        /* Set stack pointer */
        proc->context.esp = (uintptr_t)stack;
    }

    /* Setup interrupt frame */
    proc->frame.eip = (uint32_t)entry;
    proc->frame.cs = selcode;
    proc->frame.eflags = 0x202;

    proc->id = scheduler.count++;
    proc->flags = flags;

    /* Create page directory, Map kernel space into the processes address space */
    proc->context.cr3 = pgd_create_pd();
    
    /* Init memory stack ( VMM -> HEAP ) */
    if (flags & PROC_FLAG_HEAP) {
        vmm_init(&proc->vmm, UVIRT, UVIRT_END, kmalloc);
        heap_init(&proc->heap, &proc->vmm);
    }

	kdprint("[SCHEDULER] Process created pid=%u flags=%u\n", proc->id, flags);
    
    /* Append process to list */
    proc->next = NULL;
    proc->prev = scheduler.tail;
    if (scheduler.tail != NULL) {
        scheduler.tail->next = proc;
    }
    else {
        scheduler.head = proc;
    }
    scheduler.tail = proc;

    return proc;
}

static void destroy_proc(process_t* proc, int wait_self_destroy) {

    if (proc == NULL) {
        return;
    }
    
    /* Remove process from list */
    if (proc->prev != NULL) {
        proc->prev->next = proc->next;
    }
    else {
        scheduler.head = proc->next;
    }
    if (proc->next != NULL) {
        proc->next->prev = proc->prev;
    }
    else {
        scheduler.tail = proc->prev;
    }
    proc->next = NULL;
    proc->prev = NULL;

    kdprint("[SCHEDULER] Process destroyed pid=%u flags=%u\n", proc->id, proc->flags);

    if (proc->flags & PROC_FLAG_HEAP) {
        /* Destroy heap ( VMM -> HEAP ) */
        heap_destroy(&proc->heap);
        vmm_destroy(&proc->vmm, kfree);
    }

    if (proc->flags & PROC_FLAG_STACK) {
        /* Destroy stack */
        if (proc->stack != NULL) {
            kfree(proc->stack);
            proc->stack = NULL;
        }
    }

    /* Destroy page directory */
    pgd_destroy_pd(proc->context.cr3);

    if (proc->exe != NULL) {
        kfree(proc->exe);
        proc->exe = NULL;
        proc->exe_size = 0;
    }

    /* Destroy process */
    process_destroy(proc);

    if (wait_self_destroy && scheduler.current == proc) {
        /* We are unloading our self. Wait until we die */
        scheduler.current = NULL;
        halt();
    }
}

/* Process cleanup */
static void cleanup_proc(void) {
    process_t* proc = scheduler_current();
    kdprint("[SCHEDULER] Process cleanup pid=%u flags=%u\n", proc->id, proc->flags);
    scheduler_unload_kprocess(proc, 1);
}

/* Idle process */
static void idle_proc(void) {
    halt();
}

void scheduler_debug_cswitch_save(process_t* proc) {
    kdprint("[SCHEDULER] CSWITCH_SAVE pid=%u eip=%08X cs=%04X esp=%08X ss=%04X\n", proc->id, proc->frame.eip, proc->frame.cs, proc->context.esp, proc->context.ss);
}
void scheduler_debug_cswitch_load(process_t* proc) {
    kdprint("[SCHEDULER] CSWITCH_LOAD pid=%u eip=%08X cs=%04X esp=%08X ss=%04X\n", proc->id, proc->frame.eip, proc->frame.cs, proc->context.esp, proc->context.ss);
}
