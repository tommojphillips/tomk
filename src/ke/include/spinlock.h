/* spinlock.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _SPINLOCK_H
#define _SPINLOCK_H

typedef struct _spinlock {
    int locked;
} spinlock_t;

/* Aquire lock */
extern void spinlock_acquire(spinlock_t* lock);

/* Release lock */
extern void spinlock_release(spinlock_t* lock);

#endif

