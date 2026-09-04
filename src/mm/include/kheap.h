/* kheap.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KHEAP_H
#define KHEAP_H

#include <stddef.h>

/* Kernel heap initialize */
void kheap_init(void);

/* kernel memory allocate */
void* kmalloc(size_t size);

/* kernel memory free */
void kfree(void* ptr);

#endif
