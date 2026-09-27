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

typedef void* (*alloc_fn_t)(size_t);
typedef void (*free_fn_t)(void*);

static process_t* create_proc(process_entry_fn_t entry, uint32_t flags);
static void destroy_proc(process_t* proc, int wait_self_destroy);

static void idle_proc(void);
static void startup_proc(process_t* proc);

void scheduler_init(void) {
    process_t* kinit = NULL;
    process_t* idle = NULL;

    scheduler.count = 0;
    scheduler.current = NULL;
    scheduler.head = NULL;
    scheduler.tail = NULL;

    /* Create the kernel process */
    kinit = create_proc(NULL, PROC_FLAG_KERNEL | PROC_SHARED_ADDR_SPACE);
    if (kinit == NULL) {
        return;
    }
    
    /* Create the idle process */
    idle = create_proc(idle_proc, PROC_FLAG_KERNEL | PROC_SHARED_ADDR_SPACE | PROC_FLAG_STACK);
    if (idle == NULL) {
        return;
    }

    /* Set the current process to (this) kernel process */
    scheduler.current = kinit;

    /* Setup timer */
    pit_set_handler(cswitch_handler);
    pit_set_freq(1000); /* 1000Hz = 1ms period */
    pit_enable();

	kprint("[SCHEDULER] Init OK\n");
}
process_t* scheduler_load_kprocess(process_entry_fn_t entry) {
    /* Create process */
    return create_proc(entry, PROC_FLAG_KERNEL | PROC_SHARED_ADDR_SPACE | PROC_FLAG_STACK | PROC_FLAG_HEAP);
}
process_t* scheduler_load_uprocess(process_entry_fn_t entry) {
    /* Create process */
    return create_proc(entry, PROC_FLAG_USER | PROC_SHARED_ADDR_SPACE | PROC_FLAG_STACK | PROC_FLAG_HEAP);
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
        if (scheduler.head == NULL) {
            return 0; /* no processes in list, no switch */
        }
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
    alloc_fn_t alloc = NULL;

    switch (flags & PROC_FLAG_TYPE_MASK) {
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

    /* Create process */
    if (!process_create(&proc)) {
        kdprint("[SCHEDULER] Error failed to create process\n");
        return NULL;
    }

    proc->entry = entry;

    /* Set general registers */
    proc->context.eax = 0;
    proc->context.edx = 0;
    proc->context.ecx = 0;
    proc->context.ebx = 0;
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

    proc->id = scheduler.count++;
    proc->flags = flags;

    if (flags & PROC_SHARED_ADDR_SPACE) {
        proc->context.cr3 = getcr3();
    }
    else {
        /* Create page directory, Map kernel space into the processes address space */
        proc->context.cr3 = pgd_create_pd();
    }
    
    /* Init memory stack ( VMM -> HEAP ) */
    if (flags & PROC_FLAG_HEAP) {
        vmm_init(&proc->vmm, UVIRT, UVIRT_END, kmalloc);
        heap_init(&proc->heap, &proc->vmm);
    }

	kdprint("[SCHEDULER] Process created pid=%u flags=%u\n", proc->id, flags);

    /* Append process to list */
    int critsec = critsec_enter();
    proc->next = NULL;
    proc->prev = scheduler.tail;
    if (scheduler.tail != NULL) {
        scheduler.tail->next = proc;
    }
    else {
        scheduler.head = proc;
    }
    scheduler.tail = proc;
    critsec_leave(critsec);

    return proc;
}
static void destroy_proc(process_t* proc, int wait_self_destroy) {
    process_t* current = scheduler.current;
    free_fn_t free = NULL;
    
    if (proc == NULL) {
        return;
    }
    
    switch (proc->flags & PROC_FLAG_TYPE_MASK) {
        case PROC_FLAG_KERNEL:
            free = kfree;
            break;
        case PROC_FLAG_USER:
            free = kfree;
            break;
    }

    /* Remove process from list */
    int critsec = critsec_enter();
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
    if (current == proc) {
        scheduler.current = NULL;
    }
    critsec_leave(critsec);

    kdprint("[SCHEDULER] Process destroyed pid=%u flags=%u\n", proc->id, proc->flags);

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

    /* If we are unloading our self, wait until we die */
    if (wait_self_destroy && current == proc) {
        halt();
    }
}

/* Process startup */
static void startup_proc(process_t* proc) {
    kdprint("[SCHEDULER] Process startup pid=%u flags=%u\n", proc->id, proc->flags);
    proc->entry();
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
