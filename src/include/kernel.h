/* kernel.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KERNEL_H
#define KERNEL_H

/* Kernel Virtual Load Address */
#define KVIRT 0xC0000000

/* Convert Virtual address to Physical address */
#define V2P(x) ((x) - KVIRT)

/* Convert Physical address to Virtual address */
#define P2V(x) (KVIRT + (x))

/* Kernel stack base */
extern uintptr_t kstack_base;

/* Kernel stack top */
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
