/* kdprint.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KDPRINT_H
#define KDPRINT_H

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

#endif