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

/* Scheduler load kernel process */
process_t* scheduler_load_kprocess_elf(uint8_t* data, size_t size);

/* Scheduler unload kernel process */
void scheduler_unload_kprocess(process_t* proc);

/* Scheduler get current process */
process_t* scheduler_get_current_process(void);

/* Scheduler get 0th process */
process_t* scheduler_get_head(void);

#endif
