/* libc/string/strcmp.c */

#include <stddef.h>
#include <string.h>

int strcmp(const char* restrict s1, const char* restrict s2) {
    size_t l1 = strlen(s1);
    size_t l2 = strlen(s2);

    if (l1 != l2) {
        return 1;
    }

	for (size_t i = 0; i < l1; ++i) {
		if (s1[i] != s2[i]) {
            return 1;
        }
	}
	return 0;
}
int strncmp(const char* restrict s1, const char* restrict s2, size_t size) {
    size_t l1 = strlen(s1);
    size_t l2 = strlen(s2);
    size_t l = l1 < l2 ? l1 : l2 < size ? l2 : size;

	for (size_t i = 0; i < l; ++i) {
		if (s1[i] != s2[i]) {
            return 1;
        }
	}
	return 0;
}
