/* libc/string/memcpy.c */

#include <stddef.h>

int memcpy(void* restrict dest, void* restrict src, size_t count) {
	if (dest < src) {
		for (size_t i = count; i != 0; --i) {
			((char*)dest)[i - 1] = ((char*)src)[i - 1];
		}
	}
	else {
		for (size_t i = 0; i < count; ++i) {
			((char*)dest)[i] = ((char*)src)[i];
		}		
	}
	return 0;
}