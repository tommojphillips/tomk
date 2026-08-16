/* libc/include/stdio.h */

#ifndef LIBC_STDIO_H
#define LIBC_STDIO_H

#include <stddef.h>
#include <stdarg.h>

#define EOF 0

int vsprintf(char* restrict s, const char* restrict fmt, va_list args);
int vsnprintf(char* restrict s, size_t len, const char* restrict fmt, va_list args);
int vprintf(const char* restrict fmt, va_list args);
int sprintf(char* restrict buffer, const char* restrict fmt, ...);
int snprintf(char* restrict buffer, size_t buffer_size, const char* restrict fmt, ...);
int printf(const char* restrict fmt, ...);

int getchar(void);
int putchar(int ch);
int puts(const char* s);

#endif
