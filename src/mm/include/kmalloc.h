/* kmalloc.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KMALLOC_H
#define KMALLOC_H

#include <stddef.h>

/* Allocate physically contiguous memory. (Virtually contiguous and guaranteed to be physically contiguous) */
void* kmalloc(size_t size);

/* Free physically contiguous memory */
void kfree(void* ptr);

/* Allocate physically non-contiguous memory. (Virtually contiguous but not guaranteed to be physically contiguous) */
void* vmalloc(size_t size);

/* Free physically non-contiguous memory */
void vfree(void* ptr);

#endif
