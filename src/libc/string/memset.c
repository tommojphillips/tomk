/* libc/string/memset.c */

#include <stddef.h>

void* memset(void* dest, int ch, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		((char*)dest)[i] = (char)ch;
	}
	return dest;
}
