/* kernel.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdarg.h>
#include <stdio.h>

void kprintf(const char* fmt, ...) {
	va_list args;
    va_start(args, fmt);
    vfprintf(STDIO, fmt, args);
	vfprintf(SERIAL, fmt, args);
    va_end(args);
}
void kdprintf(const char* fmt, ...) {
	va_list args;
    va_start(args, fmt);
	vfprintf(SERIAL, fmt, args);
    va_end(args);
}
