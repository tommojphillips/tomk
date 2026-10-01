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
#include <critsec.h>

#define DEFAULT_STACK (1*1024*1024)

#define UVIRT     0x00001000
#define UVIRT_END KVIRT

/* Global scheduler instance */
static scheduler_t scheduler;

/* Allocate function */
typedef void* (*alloc_fn_t)(size_t);

/* Free function */
typedef void (*free_fn_t)(void*);

/* Initialize `proc` in `inactive` list. 
 Move to `active` list once fully initialized.
 proc:    Registered process in `inactive` list
 Returns: Next process in `inactive` list */
static process_t* create_proc(process_t* proc);

/* Destroy `proc` in `inactive` list
 proc:    Deregistered process in `inactive` list
 Returns: Next process in `inactive` list */
static process_t* destroy_proc(process_t* proc);

/* Process manager append process 
 `procman`: List to append `proc` from
 `proc`:    Process to append */
static void procman_append(procman_t* procman, process_t* proc);

/* Process manager remove process
 `procman`: List to remove `proc` from
 `proc`:    Process to remove */
static void procman_remove(procman_t* procman, process_t* proc);

/* Register Process for deletion */
static void proc_deregister(process_t* proc);

/* Register Process for creation
 path:  Path to executable (currently only used to identify process)
 entry: Process entry
 flags: Process flags
 Returns: The newly creately, inactive process */
static process_t* proc_register(const char* path, process_entry_fn_t entry, uint32_t flags);

/* Idle process */
static void idle_proc(void);

/* Scheduler process */
static void scheduler_proc(void);

/* Kernel process startup trampoline */
static void startup_proc(process_t* proc);

void scheduler_init(void) {
    process_t* kinit = NULL;
    process_t* idle = NULL;
    process_t* sched = NULL;

    scheduler.active.count = 0;
    scheduler.active.current = NULL;
    scheduler.active.head = NULL;
    scheduler.active.tail = NULL;

    scheduler.inactive.count = 0;
    scheduler.inactive.current = NULL;
    scheduler.inactive.head = NULL;
    scheduler.inactive.tail = NULL;

    /* Create the kernel init process */
    kinit = proc_register(".kinit", NULL, PROC_FLAG_KERNEL | PROC_SHARED_ADDR_SPACE);
    if (kinit == NULL) {
        return;
    }
    create_proc(kinit);
    
    /* Create the idle process */
    idle = proc_register(".idle", idle_proc, PROC_FLAG_KERNEL | PROC_SHARED_ADDR_SPACE | PROC_FLAG_STACK);
    if (idle == NULL) {
        return;
    }
    create_proc(idle);

    /* Create the scheduler process */
    sched = proc_register(".scheduler", scheduler_proc, PROC_FLAG_KERNEL | PROC_SHARED_ADDR_SPACE | PROC_FLAG_STACK | PROC_FLAG_HEAP);
    if (sched == NULL) {
        return;
    }
    create_proc(sched);

    /* Set the current process to (this) kernel process */
    scheduler.active.current = kinit;

    /* Setup timer */
    pit_set_handler(cswitch_handler);
    pit_set_freq(1000); /* 1000Hz = 1ms period */
    pit_enable();

	kprint("[SCHEDULER] Init OK\n");
}
process_t* scheduler_load_kprocess(const char* path, process_entry_fn_t entry) {
    return proc_register(path, entry, PROC_FLAG_KERNEL | PROC_FLAG_STACK | PROC_FLAG_HEAP);
}
process_t* scheduler_load_uprocess(const char* path, process_entry_fn_t entry) {
    return proc_register(path, entry, PROC_FLAG_USER | PROC_FLAG_STACK | PROC_FLAG_HEAP);
}
void scheduler_unload_process(process_t* proc) {
    proc_deregister(proc);
}
void scheduler_unload_current_process(void) {
    proc_deregister(scheduler_current());
}

process_t* scheduler_current(void) {
    return scheduler.active.current;
}
process_t* scheduler_head(void) {
    return scheduler.active.head;
}
process_t* scheduler_tail(void) {
    return scheduler.active.tail;
}

int scheduler_switch(process_t** current, process_t** next) {

    if (scheduler.active.current == NULL) {
        if (scheduler.active.head == NULL) {
            return 0; /* active list is empty, no switch */
        }
        scheduler.active.current = scheduler.active.head;
    }

    if (!process_update(scheduler.active.current)) {
        *current = NULL;
        *next = NULL;
        return 0; /* no switch */
    }

    *current = scheduler.active.current;
    
    if (scheduler.active.current->next != NULL) {
        *next = scheduler.active.current->next;
    }
    else {
        *next = scheduler.active.head;
    }

    scheduler.active.current = *next;

    return *next != *current; /* switch? */
}

static process_t* create_proc(process_t* proc) {
    uint16_t selcode = 0;
    uint16_t selstack = 0;
    uint16_t seldata = 0;
    alloc_fn_t alloc = NULL;
    
    proc->flags &= ~PROC_FLAG_CREATE;
    switch (proc->flags & PROC_FLAG_TYPE_MASK) {
        case PROC_FLAG_KERNEL:
            selcode = KCODE;
            selstack = KSTACK;
            seldata = KDATA;
            alloc = kmalloc;
            break;
        case PROC_FLAG_USER:
            selcode = UCODE | 3;
            selstack = USTACK | 3;
            seldata = UDATA | 3;
            alloc = kmalloc;
            break;
    }
    
    /* Set segment registers */
    proc->context.es = seldata;
    proc->context.ds = seldata;
    proc->context.fs = seldata;
    proc->context.gs = seldata;
    proc->context.ss = selstack;
    
    if (proc->flags & PROC_FLAG_STACK) {
        /* Setup stack */
        proc->stack = alloc(DEFAULT_STACK);
        if (proc->stack == NULL) {
            kdprint("[SCHEDULER] Error failed allocate process stack\n");
            return NULL;
        }
        uintptr_t* stack = (uintptr_t*)((uint8_t*)proc->stack + DEFAULT_STACK);

        /* Arg0 */
        stack--;
        *stack = (uintptr_t)proc;
        
        /* Return address */
        stack--;
        *stack = 0;

        /* Set stack pointer */
        proc->context.esp = (uintptr_t)stack;
        proc->context.ebp = (uintptr_t)stack;
    }
    else {
        proc->context.esp = 0;
        proc->context.ebp = 0;
        proc->stack = NULL;
    }

    /* Setup interrupt frame */
    proc->frame.eip = (uint32_t)startup_proc;
    proc->frame.cs = selcode;
    proc->frame.eflags = 0x202;

    proc->id = scheduler.active.count++;

    if (proc->flags & PROC_SHARED_ADDR_SPACE) {
        proc->context.cr3 = V2P((uint32_t)pg_pd);
    }
    else {
        /* Create page directory, Map kernel space into the processes address space */
        proc->context.cr3 = pgd_create_pd();
    }
    
    /* Init memory stack ( VMM -> HEAP ) */
    if (proc->flags & PROC_FLAG_HEAP) {
        vmm_init(&proc->vmm, UVIRT, UVIRT_END, kmalloc);
        heap_init(&proc->heap, &proc->vmm);
    }

    process_t* next = proc->next;
    
    /* Remove process from inactive list */
    procman_remove(&scheduler.inactive, proc);

    /* Append process to active list */
    procman_append(&scheduler.active, proc);

	kdprint("[SCHEDULER] Process created. path=\"%s\" pid=%u flags=0x%02X\n", proc->path, proc->id, proc->flags);
    return next;
}
static process_t* destroy_proc(process_t* proc) {
    process_t* next = NULL;
    free_fn_t free = NULL;
    
    if (proc == NULL) {
        return NULL;
    }

    next = proc->next;

    switch (proc->flags & PROC_FLAG_TYPE_MASK) {
        case PROC_FLAG_KERNEL:
            free = kfree;
            break;
        case PROC_FLAG_USER:
            free = kfree;
            break;
    }

    /* Remove process from list */
    procman_remove(&scheduler.inactive, proc);

    kdprint("[SCHEDULER] Process destroyed path=\"%s\" pid=%u flags=0x%02X\n", proc->path, proc->id, proc->flags);

    if (proc->flags & PROC_FLAG_HEAP) {
        /* Destroy heap ( VMM -> HEAP ) */
        heap_destroy(&proc->heap);
        vmm_destroy(&proc->vmm, kfree);
    }

    if (proc->flags & PROC_FLAG_STACK) {
        /* Destroy stack */
        if (proc->stack != NULL) {
            free(proc->stack);
            proc->stack = NULL;
        }
    }

    if (!(proc->flags & PROC_SHARED_ADDR_SPACE)) {
        /* Destroy page directory */
        pgd_destroy_pd(proc->context.cr3);
    }

    if (proc->exe != NULL) {
        free(proc->exe);
        proc->exe = NULL;
        proc->exe_size = 0;
    }

    /* Destroy process */
    process_destroy(proc);

    return next;
}

/* Process startup */
static void startup_proc(process_t* proc) {
    kdprint("[SCHEDULER] Process startup path=\"%s\" pid=%u flags=0x%02X\n", proc->path, proc->id, proc->flags);
    proc->entry();
    kdprint("[SCHEDULER] Process cleanup path=\"%s\" pid=%u flags=0x%02X\n", proc->path, proc->id, proc->flags);
    scheduler_unload_process(proc);
    halt();
}

/* Debug */
void scheduler_debug_cswitch_save(process_t* proc) {
    kdprint("[SCHEDULER] CSWITCH_SAVE path=\"%s\" pid=%u eip=%08X cs=%04X esp=%08X ss=%04X\n", proc->path, proc->id, proc->frame.eip, proc->frame.cs, proc->context.esp, proc->context.ss);
}
void scheduler_debug_cswitch_load(process_t* proc) {
    kdprint("[SCHEDULER] CSWITCH_LOAD path=\"%s\" pid=%u eip=%08X cs=%04X esp=%08X ss=%04X\n", proc->path, proc->id, proc->frame.eip, proc->frame.cs, proc->context.esp, proc->context.ss);
}

/* Idle process */
static void idle_proc(void) {
    halt();
}

/* Scheduler process */
static void scheduler_proc(void) {
    process_t* proc = NULL;
    while (1) {
        proc = scheduler.inactive.tail;
        while (proc != NULL) {
            if (proc->flags & PROC_FLAG_CREATE) {
                proc = create_proc(proc);
            }
            else if (proc->flags & PROC_FLAG_SERVICE_MASK) {
                proc = destroy_proc(proc);
            }
            else {
               proc = proc->prev;
            }
        }
    }
}

static void procman_append(procman_t* procman, process_t* proc) {
    /* Append process to list */
    int critsec = critsec_enter();
    proc->next = NULL;
    proc->prev = procman->tail;
    if (procman->tail != NULL) {
        procman->tail->next = proc;
    }
    else {
        procman->head = proc;
    }
    procman->tail = proc;
    critsec_leave(critsec);
}
static void procman_remove(procman_t* procman, process_t* proc) {
    /* Remove process from list */
    int critsec = critsec_enter();
    if (proc->prev != NULL) {
        proc->prev->next = proc->next;
    }
    else {
        procman->head = proc->next;
    }
    if (proc->next != NULL) {
        proc->next->prev = proc->prev;
    }
    else {
        procman->tail = proc->prev;
    }
    proc->next = NULL;
    proc->prev = NULL;
    if (procman->current == proc) {
        procman->current = NULL;
    }
    critsec_leave(critsec);
}

static process_t* proc_register(const char* path, process_entry_fn_t entry, uint32_t flags) {
    process_t* proc = NULL;

    /* Create process */
    if (!process_create(&proc)) {
        kdprint("[SCHEDULER] Error failed to register process creation. path=\"%s\" flags=0x%02X\n", path, flags);
        return NULL;
    }

    proc->entry = entry;
    proc->path = path;
    proc->flags = flags | PROC_FLAG_CREATE;

    /* Add process to inactive list */
    procman_append(&scheduler.inactive, proc);
    
    kdprint("[SCHEDULER] Process registered. path=\"%s\" flags=0x%02X\n", proc->path, proc->flags);
    return proc;
}
static void proc_deregister(process_t* proc) {

    if (proc == NULL) {
        return;
    }

    if (proc->flags & PROC_FLAG_KILL) {
        return;
    }

    /* Remove process from active list */
    procman_remove(&scheduler.active, proc);

    /* Add process to inactive list */
    procman_append(&scheduler.inactive, proc);

    proc->flags |= PROC_FLAG_KILL;

    kdprint("[SCHEDULER] Process deregistered. path=\"%s\" pid=%u flags=0x%02X\n", proc->path, proc->id, proc->flags);
}
