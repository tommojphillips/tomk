/* kheap.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KHEAP_H
#define KHEAP_H

#include <stddef.h>
#include <stdint.h>

#include <vmm.h>

typedef struct heap_block_t heap_block_t;
typedef struct heap_region_t heap_region_t;

struct heap_block_t {
    size_t size;
    uint32_t flags;

    heap_block_t* next;
    heap_block_t* prev;
};

struct heap_region_t {
    size_t size;
    uint32_t flags;

    heap_region_t* next;
    heap_region_t* prev;

    heap_block_t* head;
    heap_block_t* tail;
};

typedef struct heap_t {
    heap_region_t* head;
    heap_region_t* tail;
    vmm_t* vmm;
} heap_t;

/* Heap initialize */
void heap_init(heap_t* heap, vmm_t* vmm);

/* Heap destroy */
void heap_destroy(heap_t* heap);

/* Heap alloc */
void* heap_alloc(heap_t* heap, size_t size, unsigned int contiguous);

/* Heap free */
void heap_free(heap_t* heap, void* ptr);

/* Heap get free memory (in 4kb pages) */
size_t heap_get_free(heap_t* heap);

/* Heap get total memory (in 4kb pages) */
size_t heap_get_total(heap_t* heap);

/* Heap get used memory (in 4kb pages) */
size_t heap_get_used(heap_t* heap);

/* Heap get usable memory (in 4kb pages) */
size_t heap_get_usable(heap_t* heap);

#endif
