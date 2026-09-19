/* process.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

 #include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <process.h>
#include <kmalloc.h>

/* Allocate and add process to scheduling list */
int process_create(procman_t* man, process_t** proc) {
    if (man == NULL || proc == NULL) {
        return 0;
    }
    
    *proc = kmalloc(sizeof(process_t));
    if (*proc == NULL) {
        return 0;
    }
    memset(*proc, 0, sizeof(process_t));

    /* Append process to list */
    (*proc)->next = NULL;
    (*proc)->prev = man->tail;

    if (man->tail != NULL) {
        man->tail->next = *proc;
    }
    else {
        man->head = *proc;
    }

    man->tail = *proc;
    
    return 1; /* Success */
}

/* Remove and free process from scheduling list  */
void process_destroy(procman_t* man, process_t* proc) {
    if (man == NULL || proc == NULL) {
        return;
    }

    /* Remove process from list */
    if (proc->prev != NULL) {
        proc->prev->next = proc->next;
    }
    else {
        man->head = proc->next;
    }

    if (proc->next != NULL) {
        proc->next->prev = proc->prev;
    }
    else {
        man->tail = proc->prev;
    }

    proc->next = NULL;
    proc->prev = NULL;

    kfree(proc);
}

/* Select the next process */
process_t* process_next(procman_t* man, process_t* proc) {
    if (man == NULL) {
        return NULL;
    }

    if (proc == NULL || proc->next == NULL)  {
        return man->head;
    }

    return proc->next;
}

/* Update process ticks, check if a process switch is needed */
int process_update(process_t* proc) {
    if (proc == NULL) {
        return 1; /* switch process */
    }
    
    proc->ticks++;
    if (proc->ticks < 10) {
        return 0; /* give process more time */
    }
    proc->ticks = 0;
    return 1; /* switch process */
}
