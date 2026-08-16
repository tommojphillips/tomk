/* kheap.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * Kernel heap allocator
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

/* ALIGN */
#define ALIGN(t,x,a) (((t)(x) + (t)((a) - 1)) & ~((t)((a) - 1)))

#define PAGE_SIZE       0x1000
#define HEAP_GROW_PAGES 16

#define HEAP_BLOCK_FREE 0x00
#define HEAP_BLOCK_USED 0x01

typedef struct heap_block_t {
    size_t size;
    uint32_t flags;

    struct heap_block_t* next;
    struct heap_block_t* prev;
} heap_block_t;

typedef struct heap_t {
    heap_block_t* head;
    heap_block_t* tail;
} heap_t;

static heap_t heap;

static heap_block_t* heap_grow(size_t size);
static void heap_split(heap_block_t* block, size_t size);
static void heap_merge(heap_block_t* block);
static heap_block_t* heap_find(void* ptr);
static void heap_append(heap_block_t* block);
static void heap_insert_after(heap_block_t* block, heap_block_t* next);
static void heap_remove(heap_block_t* block);

void kheap_init(void) {
    heap.head = NULL;
    heap.tail = NULL;
    
    heap_block_t* block = heap_grow(HEAP_GROW_PAGES * PAGE_SIZE);

    heap_append(block);
    
    assert(heap.head != NULL);
    assert(heap.tail != NULL);

	kprint("[KHEAP] Init\n");
}

void* kmalloc(size_t size) {
    heap_block_t* block = NULL;

    if (size == 0) {
        return NULL;
    }

    size = ALIGN(size_t, size, 8);

    for (block = heap.head; block != NULL; block = block->next) {
        if (block->flags != HEAP_BLOCK_FREE) {
            continue;
        }

        if (block->size < size) {
            continue;
        }

        heap_split(block, size);

        block->flags = HEAP_BLOCK_USED;

        return (void*)(block + 1);
    }

    /* No suitable block; grow heap */
    block = heap_grow(size);
    if (block == NULL) {
        return NULL;
    }

    /* Append new region to heap */
    heap_append(block);

    heap_split(block, size);
    block->flags = HEAP_BLOCK_USED;

    //kprint("[KHEAP] alloc: %08X (%08X)\n", (uint32_t)(void*)(block + 1), block->size);
    return (void*)(block + 1);
}
void kfree(void* ptr) {
    heap_block_t* block = NULL;

    if (ptr == NULL) {
        return;
    }

    block = heap_find(ptr);

    if (block == NULL) {
        kprint("[KHEAP] Error invalid pointer: %08X\n", ptr);
        return;
    }

    if (block->flags != HEAP_BLOCK_USED) {
        kprint("[KHEAP] Error double free: %08X\n", ptr);
        return;
    }

    block->flags = HEAP_BLOCK_FREE;

    //kprint("[KHEAP] free: %08X (%08X)\n", (uint32_t)ptr, block->size);

    /* Merge with following block */
    heap_merge(block);

    /* Merge with preceding block */
    if (block->prev != NULL && block->prev->flags == HEAP_BLOCK_FREE) {
        block = block->prev;
        heap_merge(block);
    }    
}

static void heap_append(heap_block_t* block) {
    if (block == NULL) {
        return;
    }

    block->next = NULL;
    block->prev = heap.tail;

    if (heap.tail != NULL) {
        heap.tail->next = block;
    }
    else {
        heap.head = block;
    }

    heap.tail = block;
}
static void heap_insert_after(heap_block_t* block, heap_block_t* next) {
    next->prev = block;
    next->next = block->next;

    if (block->next != NULL) {
        block->next->prev = next;
    }
    else {
        heap.tail = next;
    }

    block->next = next;
}
static void heap_remove(heap_block_t* block) {
    if (block->prev != NULL) {
        block->prev->next = block->next;
    }
    else {
        heap.head = block->next;
    }

    if (block->next != NULL) {
        block->next->prev = block->prev;
    }
    else {
        heap.tail = block->prev;
    }

    block->next = NULL;
    block->prev = NULL;
}

static heap_block_t* heap_grow(size_t size) {
    size_t pages;
    size_t bytes;
    heap_block_t* block;

    pages = (size + sizeof(heap_block_t) + PAGE_SIZE - 1) >> 12;

    if (pages < HEAP_GROW_PAGES) {
        pages = HEAP_GROW_PAGES;
    }

    bytes = pages * PAGE_SIZE;
    
    block = vmm_alloc(pages);

    //kprint("[KHEAP] grow: pages=%u block=%08X vfree=%u pfree=%u\n",
    // (uint32_t)pages, (uint32_t)block, vmm_get_free(), pmm_get_free());

    if (block == NULL) {
        return NULL;
    }

    block->size = bytes - sizeof(heap_block_t);
    block->flags = HEAP_BLOCK_FREE;
    block->next = NULL;
    block->prev = NULL;

    return block;
}
static void heap_split(heap_block_t* block, size_t size) {
    heap_block_t* next = NULL;

    if (block == NULL) {
        return;
    }

    /* Dont create a useless tiny block */
    if (block->size < size + sizeof(heap_block_t) + 16) {
        return;
    }

    next = (heap_block_t*)((uint8_t*)(block + 1) + size);
    next->size = block->size - size - sizeof(heap_block_t);
    next->flags = HEAP_BLOCK_FREE;

    heap_insert_after(block, next);
    block->size = size;
}
static void heap_merge(heap_block_t* block) {
    heap_block_t* next = NULL;

    if (block == NULL) {
        return;
    }

    next = block->next;

    if (next == NULL || next->flags != HEAP_BLOCK_FREE) {
        return;
    }

    /* Blocks must be adjacent in virtual address space */
    if ((uint8_t*)block + sizeof(heap_block_t) + block->size != (uint8_t*)next) {
        return;
    }

    block->size += sizeof(heap_block_t) + next->size;
    heap_remove(next);
}
static heap_block_t* heap_find(void* ptr) {
    for (heap_block_t* block = heap.head; block != NULL; block = block->next) {
        if ((void*)(block + 1) == ptr) {
            return block;
        }
    }
    return NULL;
}
