/* scheduler.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <stdint.h>
#include <stddef.h>

#include <process.h>

typedef struct scheduler_t {
    procman_t procman;
} scheduler_t;

/* Scheduler Initialize */
void scheduler_init(void);

/* Scheduler load kernel process */
process_t* scheduler_load_kprocess(uintptr_t entry);

/* Scheduler unload kernel process */
void scheduler_unload_kprocess(process_t* proc, int ret);

/* Scheduler load user process */
process_t* scheduler_load_uprocess(uintptr_t entry);

/* Scheduler unload user process */
void scheduler_unload_uprocess(process_t* proc, int ret);

/* Scheduler get current process */
process_t* scheduler_current(void);

/* Scheduler get first loaded process */
process_t* scheduler_head(void);

/* Scheduler get last loaded process */
process_t* scheduler_tail(void);

#endif
