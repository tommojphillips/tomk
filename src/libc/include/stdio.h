/* libc/include/stdio.h */

#ifndef LIBC_STDIO_H
#define LIBC_STDIO_H

#include <stdint.h>
#include <stdarg.h>

#define EOF 0

int printf(const char* restrict fmt, ...);
int getchar(void);
int putchar(int ch);
int puts(const char* s);

#endif