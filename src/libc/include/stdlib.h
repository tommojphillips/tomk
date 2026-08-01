/* libc/include/stdlib.h */

#ifndef LIBC_STDLIB_H
#define LIBC_STDLIB_H

#include <stddef.h>

void* malloc(size_t size);
void free(void* s);

#endif