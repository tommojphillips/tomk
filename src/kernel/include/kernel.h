/* kernel.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KERNEL_H
#define KERNEL_H

#define KDBG
#ifdef KDBG
#include <stdio.h>
#define kprint(...) printf(__VA_ARGS__)
#else
#define kprint(...)
#endif

void kernel_panic(const char* msg);
void kernel_hang(void);

#endif
