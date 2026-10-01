/* process.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

 #include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <process.h>
#include <kmalloc.h>

/* Allocate and add process to scheduling list */
int process_create(process_t** proc) {
    if (proc == NULL) {
        return 0;
    }
    
    *proc = kmalloc(sizeof(process_t));
    if (*proc == NULL) {
        return 0;
    }
    memset(*proc, 0, sizeof(process_t));
    
    return 1; /* Success */
}

/* Remove and free process from scheduling list  */
void process_destroy(process_t* proc) {
    if (proc == NULL) {
        return;
    }

    kfree(proc);
}

/* Update process ticks, check if a process switch is needed */
int process_update(process_t* proc) {
    if (proc == NULL) {
        return 1; /* switch process */
    }
    
    proc->ticks++;
    if (proc->ticks < 2) {
        return 0; /* give process more time */
    }
    proc->ticks = 0;
    return 1; /* switch process */
}
