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
#include <align.h>
#include <paging.h>

#include <assert.h>
#include <kdprint.h>
#include <kernel.h>

#define GROW_PAGES           0x10

#define BLOCK_FREE           0x00 /* free = 0; used = 1 */
#define BLOCK_USED           0x01 /* free = 0; used = 1 */

#define REGION_NONCONTIGUOUS 0x00 /* non-contiguous physical pages = 0; contiguous physical pages = 1 */
#define REGION_CONTIGUOUS    0x02 /* non-contiguous physical pages = 0; contiguous physical pages = 1 */

#define BLOCK_ALIGNMENT      0x10
#define MIN_BLOCK_SIZE       0x10

#define HEADER_SIZE          ALIGN(size_t, sizeof(heap_block_t), BLOCK_ALIGNMENT)
#define REGION_SIZE          ALIGN(size_t, sizeof(heap_region_t), BLOCK_ALIGNMENT)

/* Get block pointer from block */
#define BLOCK_PTR(block)     ((void*)((uint8_t*)(block) + HEADER_SIZE))

/* Get block header pointer from region */
#define REGION_BLOCK(region) ((heap_block_t*)((uint8_t*)(region) + REGION_SIZE))

void heap_init(heap_t* heap, vmm_t* vmm) {
    assert(heap != NULL);
    assert(vmm != NULL);

    heap->head = NULL;
    heap->tail = NULL;
    heap->vmm = vmm;
}
void heap_destroy(heap_t* heap) {
    assert(heap != NULL);

    heap_region_t* region = NULL;
    heap_block_t* block = NULL;

    for (region = heap->head; region != NULL; region = region->next) {
        for (block = region->head; block != NULL; block = block->next) {
            
            if (!(block->flags & BLOCK_USED)) {
                continue;
            }

        }
    }

    heap->head = NULL;
    heap->tail = NULL;
    heap->vmm = NULL;
}

size_t heap_get_total(heap_t* heap) {
    return vmm_get_total(heap->vmm);
}
size_t heap_get_free(heap_t* heap) {
    return vmm_get_free(heap->vmm);
}
size_t heap_get_used(heap_t* heap) {
    return vmm_get_used(heap->vmm);
}
size_t heap_get_usable(heap_t* heap) {
    return vmm_get_usable(heap->vmm);
}

static void region_append(heap_t* heap, heap_region_t* region) {
    if (region == NULL) {
        return;
    }

    region->next = NULL;
    region->prev = heap->tail;

    if (heap->tail != NULL) {
        heap->tail->next = region;
    }
    else {
        heap->head = region;
    }

    heap->tail = region;
}
static void region_remove(heap_t* heap, heap_region_t* region) {
    if (region == NULL) {
        return;
    }

    if (region->prev != NULL) {
        region->prev->next = region->next;
    }
    else {
        heap->head = region->next;
    }

    if (region->next != NULL) {
        region->next->prev = region->prev;
    }
    else {
        heap->tail = region->prev;
    }

    region->next = NULL;
    region->prev = NULL;
}
static void block_insert_after(heap_region_t* region, heap_block_t* block, heap_block_t* next) {
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
static void block_remove(heap_region_t* region, heap_block_t* block) {
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
static int block_alloc(heap_region_t* region, heap_block_t* block, size_t size) {
    /* obtain a BLOCK */
    heap_block_t* next = NULL;

    if (region == NULL) {
        return 0;
    }

    if (block == NULL) {
        return 0;
    }

    if (block->size < size) {
        return 0;
    }

    /* Split block if enough space remains for another block */
    if (block->size >= size + HEADER_SIZE + MIN_BLOCK_SIZE) {        
        next = (heap_block_t*)((uint8_t*)block + HEADER_SIZE + size);
        next->size = block->size - HEADER_SIZE - size;
        next->flags = BLOCK_FREE;
        next->next = NULL;
        next->prev = NULL;
        
        block_insert_after(region, block, next);
        block->size = size;
        kdprint("[HEAP] block_alloc: region=%X block=%X size=%X next=%X\n", region, block, block->size, next);
    }
    block->flags |= BLOCK_USED;
    return 1;
}
static void block_free(heap_region_t* region, heap_block_t* block) {
    heap_block_t* next = NULL;

    if (region == NULL) {
        return;
    }

    if (block == NULL) {
        return;
    }

    next = block->next;

    /* Nothing to free */
    if (next == NULL) {
        return;
    }

    /* Dont free used blocks */
    if (next->flags & BLOCK_USED) {
        return;
    }

    block->size += HEADER_SIZE + next->size;

    kdprint("[HEAP] block_free: region=%X block=%X size=%X\n", region, block, block->size);

    block_remove(region, next);
}
static heap_block_t* block_find(heap_t* heap, void* ptr, heap_region_t** out_region) {
    heap_region_t* region;
    heap_block_t* block;

    /* Find block in regions */
    for (region = heap->head; region != NULL; region = region->next) {
        for (block = region->head; block != NULL; block = block->next) {
            if (BLOCK_PTR(block) == ptr) {
                if (out_region != NULL) {
                    *out_region = region;
                }
                return block;
            }
        }
    }
    return NULL;
}
static heap_block_t* block_find_free(heap_t* heap, size_t size, unsigned int contiguous, heap_region_t** out_region) {
    heap_region_t* region = NULL;
    heap_block_t* block = NULL;

    /* Search existing regions for a suitable free block */
    for (region = heap->head; region != NULL; region = region->next) {
        for (block = region->head; block != NULL; block = block->next) {
            
            /* Block in use ? */
            if (block->flags & BLOCK_USED) {
                continue;
            }

            /* Block big enough? */
            if (block->size < size) {
                continue;
            }

            /* Contiguous requirement? */
            if ((contiguous != 0) != ((region->flags & REGION_CONTIGUOUS) != 0)) {
                continue;
            }

            if (out_region != NULL) {
                *out_region = region;
            }
            return block;
        }
    }
    return NULL;
}
static heap_region_t* heap_grow(heap_t* heap, size_t size, unsigned int contiguous) {
    /* Obtain a REGION */
    heap_region_t* region = NULL;
    heap_block_t* block = NULL;
    size_t pages = 0;
    
    pages = TO_PAGE(size + REGION_SIZE + HEADER_SIZE + (PAGE_SIZE-1));

    /* Always ask the VMM for at min GROW_PAGES */
    if (pages < GROW_PAGES) {
        pages = GROW_PAGES;
    }
    
    if (contiguous) {
        region = vmm_alloc_contiguous(heap->vmm, pages);
        if (region == NULL) {
            return NULL;
        }
        region->flags = REGION_CONTIGUOUS;
    }
    else {
        region = vmm_alloc(heap->vmm, pages);
        if (region == NULL) {
            return NULL;
        }
        region->flags = REGION_NONCONTIGUOUS;
    }

    block = REGION_BLOCK(region);
    block->size = TO_ADDR(pages) - REGION_SIZE - HEADER_SIZE;
    block->flags = BLOCK_FREE;
    block->next = NULL;
    block->prev = NULL;

    region->size = TO_ADDR(pages);
    region->head = block;
    region->tail = block;
    region->next = NULL;
    region->prev = NULL;

    kdprint("[HEAP] heap_grow: region=%X size=%X pages=%d end=%X contiguous=%d\n", region, region->size, pages, (uint8_t*)region + region->size, contiguous);

    return region;
}
static void heap_shrink(heap_t* heap, heap_region_t* region, heap_block_t* block) {
    
    /* Merge with following block */
    block_free(region, block);

    /* Merge with preceding block */
    if (block->prev != NULL && !(block->prev->flags & BLOCK_USED)) {
        block = block->prev;
        block_free(region, block);
    }

    /* If this region now contains exactly one free block, return the region to the VMM */
    if (region->head == region->tail && region->head == block && !(block->flags & BLOCK_USED)) {
        size_t page_count = TO_PAGE(region->size);
        void* virt_addr = region;

        kdprint("[HEAP] heap_shrink: region=%X size=%X contiguous=%d\n", virt_addr, region->size, (region->flags & REGION_CONTIGUOUS));

        region_remove(heap, region);
        vmm_free(heap->vmm, virt_addr, page_count);
    }
}

void* heap_alloc(heap_t* heap, size_t size, unsigned int contiguous) {
    heap_region_t* region = NULL;
    heap_block_t* block = NULL;

    if (heap == NULL) {
        return NULL;
    }

    if (size == 0) {
        return NULL;
    }

    /* Blocks always start off aligned to a page boundary. We 
     make sure to align the size?!? to always keep the next 
     allocation in this block aligned to atleast BLOCK_ALIGNMENT --
     --seems wrong. shouldnt i just keep BLOCK_PTR aligned to BLOCK_ALIGNMENT? 
     There would be a reason; not sure what i was thinking because i didnt fucking comment it. */
    size = ALIGN(size_t, size, BLOCK_ALIGNMENT);

    /* Search existing regions for a suitable free block */
    block = block_find_free(heap, size, contiguous, &region);
    if (block != NULL) {
        /* Allocate from the new free region */
        if (block_alloc(region, block, size)) {
            return BLOCK_PTR(block);
        }
    }

    /* No suitable block; Create new region */
    region = heap_grow(heap, size, contiguous);
    if (region == NULL) {
        return NULL;
    }

    /* Append new region to heap */
    region_append(heap, region);
    block = region->head;

    /* Allocate from the new free region */
    if (block_alloc(region, block, size)) {
        return BLOCK_PTR(block);
    }
    return NULL;
}
void heap_free(heap_t* heap, void* ptr) {
    heap_region_t* region = NULL;
    heap_block_t* block = NULL;

    if (heap == NULL) {
        return;
    }

    if (ptr == NULL) {
        return;
    }

    block = block_find(heap, ptr, &region);
    if (block == NULL) {
        kprint("[HEAP] Error invalid pointer: %08X\n", ptr);
        return;
    }
    
    if (!(block->flags & BLOCK_USED)) {
        kprint("[HEAP] Error double free: %08X\n", ptr);
        return;
    }

    block->flags &= ~BLOCK_USED;
    
    /* Release memory not in use by heap */
    heap_shrink(heap, region, block);
}
