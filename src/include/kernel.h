/* kernel.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KERNEL_H
#define KERNEL_H

/* Kernel stack base */
extern uintptr_t kstack_base;

/* Kernel stack top */
extern uintptr_t kstack_top;

/* Hang system */
void khang(void);

#endif
