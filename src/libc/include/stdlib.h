/* stdlib.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef LIBC_STDLIB_H
#define LIBC_STDLIB_H

#include <stddef.h>

void* malloc(size_t size);
void free(void* s);

unsigned long strtoul(const char* restrict str, char** restrict end_ptr, int radix);
unsigned long long strtoull(const char* restrict str, char** restrict end_ptr, int radix);

long strtol(const char* restrict str, char** restrict end_ptr, int radix);
long long strtoll(const char* restrict str, char** restrict end_ptr, int radix);

int atoi(const char* str);
long atol(const char* str);
long long atoll(const char* str);

#endif
