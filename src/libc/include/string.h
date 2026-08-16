/* libc/include/string.h */

#ifndef LIBC_STRING_H
#define LIBC_STRING_H

#include <stddef.h>

int memcpy(void* restrict dest, void* restrict src, size_t length);
void* memset(void* dest, int ch, size_t count);
char* strcpy(char* restrict dest, const char* restrict src);
size_t strlen(const char* str);

int strcmp(const char* s1, const char* s2);

#endif
