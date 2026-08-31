/* kernel.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KERNEL_H
#define KERNEL_H

/* Kernel Virtual Load Address */
#define KVIRT 0xC0000000

#define KDBG
#ifdef KDBG
#include <stdio.h>
#include <stdarg.h>
#define kprint(...) kprintf(__VA_ARGS__)
#define kdprint(...) kdprintf(__VA_ARGS__)
#else
#define kprint(...)
#define kdprint(...)
#endif

/* Print to stdout and serial */
void kprintf(const char* fmt, ...);

/* Print to serial */
void kdprintf(const char* fmt, ...);

/* Panic (hang system) */
void kernel_panic(const char* fmt, ...);

/* Hang system */
void kernel_hang(void);

#endif
