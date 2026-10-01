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
    procman_t active;
    procman_t inactive;
} scheduler_t;

/* Scheduler Initialize */
void scheduler_init(void);

/* Scheduler load kernel process 
 path:    Executable path   
 entry:   Entry point
 Returns: Inactive kernel process */
process_t* scheduler_load_kprocess(const char* path, process_entry_fn_t entry);

/* Scheduler load user process 
 path: Executable path 
 entry: Entry point
 Returns: Inactive user process */
process_t* scheduler_load_uprocess(const char* path, process_entry_fn_t entry);

/* Scheduler unload process 
 proc: The process to unload */
void scheduler_unload_process(process_t* proc);

/* Scheduler unload currently scheduled process */
void scheduler_unload_current_process(void);

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
