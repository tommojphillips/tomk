/* kheap.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * Kernel free list heap allocator
 * The heap obtains virtual memory in pages from the VMM and subdivides
 * those regions into variable-sized blocks. Each block contains a header
 * used to track its size and allocation state.
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#include <kheap.h>
#include <vmm.h>
#include <kernel.h>
#include <assert.h>
#include <align.h>

#define PAGE_SIZE         0x1000

#define HEAP_GROW_PAGES   0x10

#define HEAP_BLOCK_USED   0x01 /* free = 0; used = 1 */
#define BLOCK_ALIGNMENT   0x10

#define HEAP_HEADER_SIZE        ALIGN(size_t, sizeof(heap_block_t), BLOCK_ALIGNMENT)
#define HEAP_REGION_SIZE        ALIGN(size_t, sizeof(heap_region_t), BLOCK_ALIGNMENT)

/* Get block pointer from block */
#define HEAP_BLOCK_PTR(block)   ((void*)((uint8_t*)(block) + HEAP_HEADER_SIZE))

/* Get block header pointer from region */
#define HEAP_REGION_BLOCK(region) ((heap_block_t*)((uint8_t*)(region) + HEAP_REGION_SIZE))

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

    heap_region_t* next;
    heap_region_t* prev;

    heap_block_t* head;
    heap_block_t* tail;
};

typedef struct heap_t {
    heap_region_t* head;
    heap_region_t* tail;
} heap_t;

static heap_t heap;

static heap_region_t* heap_grow(size_t size);
static void heap_split(heap_region_t* region, heap_block_t* block, size_t size);
static void heap_merge(heap_region_t* region, heap_block_t* block);
static heap_block_t* heap_find(void* ptr, heap_region_t** out_region);

static void heap_region_append(heap_region_t* region);
static void heap_region_remove(heap_region_t* region);
static void heap_block_insert_after(heap_region_t* region, heap_block_t* block, heap_block_t* next);
static void heap_block_remove(heap_region_t* region, heap_block_t* block);

void kheap_init(void) {
    heap.head = NULL;
    heap.tail = NULL;
}

void* kmalloc(size_t size) {
    heap_region_t* region = NULL;
    heap_block_t* block = NULL;

    if (size == 0) {
        return NULL;
    }

    size = ALIGN(size_t, size, 8);

    /* Search existing regions for a suitable free block */
    for (region = heap.head; region != NULL; region = region->next) {
        for (block = region->head; block != NULL; block = block->next) {
            
            if (block->flags & HEAP_BLOCK_USED) {
                continue;
            }

            if (block->size < size) {
                continue;
            }

            heap_split(region, block, size);
            block->flags |= HEAP_BLOCK_USED;
            return HEAP_BLOCK_PTR(block);
        }
    }

    /* No suitable block; Create new region */
    region = heap_grow(size);
    if (region == NULL) {
        return NULL;
    }

    /* Append new region to heap */
    heap_region_append(region);
    block = region->head;

    /* Allocate from the new free region */
    heap_split(region, block, size);
    block->flags |= HEAP_BLOCK_USED;
    return HEAP_BLOCK_PTR(block);
}
void kfree(void* ptr) {
    heap_region_t* region = NULL;
    heap_block_t* block = NULL;

    if (ptr == NULL) {
        return;
    }

    block = heap_find(ptr, &region);
    if (block == NULL) {
        kprint("[KHEAP] Error invalid pointer: %08X\n", ptr);
        return;
    }

    if (!(block->flags & HEAP_BLOCK_USED)) {
        kprint("[KHEAP] Error double free: %08X\n", ptr);
        return;
    }
    
    block->flags &= ~HEAP_BLOCK_USED;
    
    /* Merge with following block */
    heap_merge(region, block);

    /* Merge with preceding block */
    if (block->prev != NULL && !(block->prev->flags & HEAP_BLOCK_USED)) {
        block = block->prev;
        heap_merge(region, block);
    }

    /* If this region now contains exactly one free block, return the region to the VMM */
    if (region->head == region->tail && region->head == block && !(block->flags & HEAP_BLOCK_USED)) {
        size_t page_count = region->size >> 12;
        void* virt_addr = region;

        kdprint("[KHEAP] release: region=%X size=%X\n", virt_addr, region->size);

        heap_region_remove(region);
        vmm_free(virt_addr, page_count);
    }
}

static heap_region_t* heap_grow(size_t size) {
    /* Obtain a REGION */
    heap_region_t* region = NULL;
    heap_block_t* block = NULL;
    size_t pages = 0;
    
    pages = (size + HEAP_REGION_SIZE + HEAP_HEADER_SIZE + PAGE_SIZE - 1) >> 12;

    if (pages < HEAP_GROW_PAGES) {
        pages = HEAP_GROW_PAGES;
    }
    
    region = vmm_alloc(pages);
    if (region == NULL) {
        return NULL;
    }

    region->size = pages << 12;
    region->head = NULL;
    region->tail = NULL;    
    region->next = NULL;
    region->prev = NULL;

    block = HEAP_REGION_BLOCK(region);
    block->size = region->size - HEAP_REGION_SIZE - HEAP_HEADER_SIZE;
    block->flags = 0;
    block->next = NULL;
    block->prev = NULL;

    region->head = block;
    region->tail = block;

    kdprint("[KHEAP] grow: region=%X size=%X pages=%d end=%X\n", region, region->size, pages, (uint8_t*)region + region->size);

    return region;
}
static void heap_split(heap_region_t* region, heap_block_t* block, size_t size) {
    /* obtain a BLOCK */
    heap_block_t* next = NULL;

    if (region == NULL) {
        return;
    }

    if (block == NULL) {
        return;
    }

    /* Dont create a useless tiny block */
    if (block->size < size + HEAP_HEADER_SIZE + 16) {
        return;
    }

    next = (heap_block_t*)((uint8_t*)block + HEAP_HEADER_SIZE + size);
    next->size = block->size - HEAP_HEADER_SIZE - size;
    next->flags = 0; /* HEAP_BLOCK_FREE */
    next->next = NULL;
    next->prev = NULL;

    heap_block_insert_after(region, block, next);
    block->size = size;

    kdprint("[KHEAP] split: region=%X block=%X size=%X next=%X\n", region, block, block->size, next);
}
static void heap_merge(heap_region_t* region, heap_block_t* block) {
    heap_block_t* next = NULL;

    if (region == NULL) {
        return;
    }

    if (block == NULL) {
        return;
    }

    next = block->next;

    /* Nothing to merge */
    if (next == NULL) {
        return;
    }

    /* Dont merge used blocks */
    if (next->flags & HEAP_BLOCK_USED) {
        return;
    }

    block->size += HEAP_HEADER_SIZE + next->size;

    kdprint("[KHEAP] merge: region=%X block=%X size=%X\n", region, block, block->size);

    heap_block_remove(region, next);
}
static heap_block_t* heap_find(void* ptr, heap_region_t** out_region) {
    heap_region_t* region;
    heap_block_t* block;

    for (region = heap.head; region != NULL; region = region->next) {
        for (block = region->head; block != NULL; block = block->next) {
            if (HEAP_BLOCK_PTR(block) == ptr) {
                if (out_region != NULL) {
                    *out_region = region;
                }
                return block;
            }
        }
    }
    return NULL;
}

static void heap_region_append(heap_region_t* region) {
    if (region == NULL) {
        return;
    }

    region->next = NULL;
    region->prev = heap.tail;

    if (heap.tail != NULL) {
        heap.tail->next = region;
    }
    else {
        heap.head = region;
    }

    heap.tail = region;
}
static void heap_region_remove(heap_region_t* region) {
    if (region == NULL) {
        return;
    }

    if (region->prev != NULL) {
        region->prev->next = region->next;
    }
    else {
        heap.head = region->next;
    }

    if (region->next != NULL) {
        region->next->prev = region->prev;
    }
    else {
        heap.tail = region->prev;
    }

    region->next = NULL;
    region->prev = NULL;
}

static void heap_block_insert_after(heap_region_t* region, heap_block_t* block, heap_block_t* next) {
    if (region == NULL || block == NULL || next == NULL) {
        return;
    }

    next->prev = block;
    next->next = block->next;

    if (block->next != NULL) {
        block->next->prev = next;
    }
    else {
        region->tail = next;
    }

    block->next = next;
}
static void heap_block_remove(heap_region_t* region, heap_block_t* block) {
    if (region == NULL || block == NULL) {
        return;
    }

    if (block->prev != NULL) {
        block->prev->next = block->next;
    }
    else {
        region->head = block->next;
    }

    if (block->next != NULL) {
        block->next->prev = block->prev;
    }
    else {
        region->tail = block->prev;
    }

    block->next = NULL;
    block->prev = NULL;
}
