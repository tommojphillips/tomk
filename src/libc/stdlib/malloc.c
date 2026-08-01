
#include <stdint.h>
#include <stddef.h>

void* malloc(size_t size) {
    (void)size;
    return NULL;
}
void free(void* pointer) {
    (void)pointer;
}
