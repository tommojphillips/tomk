
#include <stdint.h>
#include <stddef.h>

#ifdef LIBK
#include <kheap.h>
#endif

void* malloc(size_t size) {
#ifdef LIBK
    return kmalloc(size);
#endif
}
void free(void* ptr) {
#ifdef LIBK
    kfree(ptr);
#endif
}
