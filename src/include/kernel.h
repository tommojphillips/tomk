/* kernel.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KERNEL_H
#define KERNEL_H

/* Kernel Virtual Load Address */
#define KVIRT 0xC0000000

extern uintptr_t kstack_base;
extern uintptr_t kstack_top;

/* Kernel version; major */
extern int kver_major;

/* Kernel version; minor */
extern int kver_minor;

/* Panic (hang system) */
void kpanic(const char* fmt, ...);

/* Hang system */
void khang(void);

#endif
