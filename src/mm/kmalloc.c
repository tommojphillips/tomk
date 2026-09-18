/* kmalloc.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stddef.h>
#include <kheap.h>

extern heap_t kheap;

void* kmalloc(size_t size) {
    return heap_alloc(&kheap, size, 1);
}
void kfree(void* ptr) {
    heap_free(&kheap, ptr);
}

void* vmalloc(size_t size) {
    return heap_alloc(&kheap, size, 0);
}
void vfree(void* ptr) {
    heap_free(&kheap, ptr);
}
