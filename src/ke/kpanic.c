/* kpanic.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdarg.h>
#include <stdio.h>

#include <khang.h>

void kpanic(const char* fmt, ...) {
	const char* panic_str = "\nKERNEL PANIC\n";
	va_list args;
    va_start(args, fmt);
	fprintf(STDIO, panic_str);
	vfprintf(STDIO, fmt, args);
	fprintf(SERIAL, panic_str);
	vfprintf(SERIAL, fmt, args);
    va_end(args);

	khang();
}
