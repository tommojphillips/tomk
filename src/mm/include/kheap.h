/* kheap.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KHEAP_H
#define KHEAP_H

#include <stddef.h>

/* Kernel heap initialize */
void kheap_init(void);

/* Allocate physically contiguous memory. (Virtually contiguous and guaranteed to be physically contiguous) */
void* kmalloc(size_t size);

/* Free physically contiguous memory */
void kfree(void* ptr);

/* Allocate physically non-contiguous memory. (Virtually contiguous but not guaranteed to be physically contiguous) */
void* vmalloc(size_t size);

/* Free physically non-contiguous memory */
void vfree(void* ptr);

size_t kheap_get_free(void);
size_t kheap_get_total(void);
size_t kheap_get_used(void);
size_t kheap_get_usable(void);

#endif
