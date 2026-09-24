/* scheduler.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <stdint.h>
#include <stddef.h>

#include <process.h>

/* Scheduler struct */
typedef struct scheduler_t {
    process_t* head;
    process_t* tail;
    process_t* current;
    size_t count;
} scheduler_t;

/* Scheduler Initialize */
void scheduler_init(void);

/* Scheduler load kernel process  
 entry: Process entry point
 Returns: Loaded process */
process_t* scheduler_load_kprocess(process_entry_fn_t entry);

/* Scheduler unload kernel process 
 proc: The process to unload
 wait_self_destroy: If process is currently scheduled, spin until process is unloaded */
void scheduler_unload_kprocess(process_t* proc, int wait_self_destroy);

/* Scheduler load user process 
 entry: Process entry point
 Returns: Loaded process */
process_t* scheduler_load_uprocess(process_entry_fn_t entry);

/* Scheduler unload user process 
 proc: The process to unload
 wait_self_destroy: If process is currently scheduled, spin until process is unloaded */
void scheduler_unload_uprocess(process_t* proc, int wait_self_destroy);

/* Scheduler get current process  
 Returns: The currently scheduled process */
process_t* scheduler_current(void);

/* Scheduler get first loaded process  
 Returns: The first loaded process */
process_t* scheduler_head(void);

/* Scheduler get last loaded process  
 Returns: The last loaded process */
process_t* scheduler_tail(void);

#endif
