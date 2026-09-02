/* pmm.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * Physical address memory manager
 */

#include <stdint.h>
#include <string.h>

#include <pmm.h>
#include <kmmap.h>
#include <kalloc.h>
#include <kernel.h>
#include <assert.h>
#include <align.h>
#include <paging.h>

/* CEIL DIV */
#define CEIL_DIV(x,y) (((x) + (y) - 1) / (y))

typedef struct pmm_t {
    uint32_t* bitmap;
    size_t bitmap_size;
    size_t total_pages;
    size_t usable_pages;
    size_t free_pages;
    size_t used_pages;
    size_t hint;
    uintptr_t memory_end;
} pmm_t;

/* Physical Memory Manager */
static pmm_t pmm;

static void pmm_set(uintptr_t phys);
static void pmm_clear(uintptr_t phys);
static int pmm_test(uintptr_t phys);
static uintptr_t pmm_alloc_one(void);
static uintptr_t pmm_find_contiguous_pages(size_t count);

void pmm_init(const kmmap_t* kmmap) {
    assert(kmmap != NULL);
    assert(kmmap->count > 0);
    
    /* Find the highest usable physical address */
    pmm.memory_end = 0;
    pmm.usable_pages = 0;
    for (size_t i = 0; i < kmmap->count; ++i) {
        if (!(kmmap->regions[i].flags & KMREGION_VALID)) {
            continue;
        }
        if ((kmmap->regions[i].flags & KMREGION_TYPE_MASK) != KMREGION_TYPE_RAM) {
            continue;
        }
        
        uintptr_t end = kmmap->regions[i].address + kmmap->regions[i].size;

        if (end > pmm.memory_end) {
            pmm.memory_end = end;
        }

        uintptr_t start_page = ALIGN(uintptr_t, kmmap->regions[i].address, PAGE_SIZE);
        uintptr_t end_page = end & ~(uintptr_t)(PAGE_SIZE - 1);

        if (end_page > start_page) {
            pmm.usable_pages += PAGE_COUNT(end_page - start_page);
        }
    }
    
    pmm.total_pages = PAGE_COUNT(pmm.memory_end + (PAGE_SIZE-1));
    pmm.bitmap_size = CEIL_DIV(pmm.total_pages, 32) * sizeof(uint32_t);
    pmm.free_pages = 0;
    pmm.used_pages = pmm.usable_pages;
    pmm.hint = 0;
    
    /* Allocate memory for the bitmap */
    pmm.bitmap = kinit_alloc(pmm.bitmap_size);
    assert(pmm.bitmap != NULL);

    /* Mark all physical addresses used */
    memset(pmm.bitmap, 0xFF, pmm.bitmap_size);
    
    /* Find all usable RAM regions in kmmap and mark those pages free in PMM */
    for (size_t i = 0; i < kmmap->count; ++i) {
        if (!(kmmap->regions[i].flags & KMREGION_VALID)) {
            continue;
        }
        if ((kmmap->regions[i].flags & KMREGION_TYPE_MASK) != KMREGION_TYPE_RAM) {
            continue;
        }
        
        pmm_mark_free(kmmap->regions[i].address, kmmap->regions[i].size);
    }

    /* zero page */
    pmm_mark_used(0x00000000, 0x1000);
}

uintptr_t pmm_alloc(size_t count) {
    /* Contiguous physical addresses */

    if (count == 0) {
        return 0;
    }
    
    if (count > pmm.free_pages) {
        return 0;
    }
    
    /* If just 1 page is requested, skip the contiguous run calculations */
    if (count == 1) {
        return pmm_alloc_one();
    }
    
    /* Multiple pages have been requested. Figure out were the next contiguous block of physical pages are */
    uintptr_t start = pmm_find_contiguous_pages(count);
    if (start == 0) {
        kprint("[PMM] fragmentation error: %u\n", count);
        return 0;
    }

    /* Mark pages used */
    for (size_t i = 0; i < count; i++) {
        pmm_set((start + i) << 12);
    }

    pmm.free_pages -= count;
    pmm.used_pages += count;

    return start << 12;
}
void pmm_free(uintptr_t phys, size_t count) {    
    if (count == 0) {
        return;
    }
    
    /* Physical address must be page aligned */
    if (phys & (PAGE_SIZE - 1)) {
        kprint("[PMM] error mis-aligned page: %8.8X\n", phys);
        return;
    }
    
    uintptr_t page = PAGE_COUNT(phys);

    /* Page must be managed by PMM */
    if (page >= pmm.total_pages || count > pmm.total_pages - page) {
        kprint("[PMM] error address out of bounds: %8.8X\n", phys);
        return;
    }

    /* Validate that all pages are in use */
    for (size_t i = 0; i < count; i++) {
        if (!pmm_test(phys + i * PAGE_SIZE)) {
            kprint("[PMM] error double free page: %8.8X\n", phys + i * PAGE_SIZE);
            return;
        }
    }

    /* Mark pages free */
    for (size_t i = 0; i < count; i++) {
        pmm_clear(phys + i * PAGE_SIZE);
    }

    pmm.free_pages += count;
    pmm.used_pages -= count;
}

void pmm_mark_free(uintptr_t phys, size_t size) {    
    if (phys > pmm.memory_end) {
        return;
    }

    uintptr_t start = phys & ~(PAGE_SIZE-1);
    uintptr_t end = (phys + size + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);

    for (uintptr_t p = start; p < end; p += PAGE_SIZE) {
        uintptr_t page = PAGE_COUNT(p);
        
        if (page >= pmm.total_pages) {
            continue;
        }

        if (pmm_test(p)) {
            pmm_clear(p);
            pmm.free_pages++;
            pmm.used_pages--;
        }
    }

    kdprint("[PMM] mark free: %08X-%08X\n", start, end);
}
void pmm_mark_used(uintptr_t phys, size_t size) {    
    if (phys > pmm.memory_end) {
        return;
    }

    uintptr_t start = phys & ~(PAGE_SIZE-1);
    uintptr_t end = (phys + size + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);

    for (uintptr_t p = start; p < end; p += PAGE_SIZE) {
        uintptr_t page = PAGE_COUNT(p);

        if (page >= pmm.total_pages) {
            continue;
        }

        if (!pmm_test(p)) {
            pmm_set(p);
            pmm.free_pages--;
            pmm.used_pages++;
        }
    }

    kdprint("[PMM] mark used: %08X-%08X\n", start, end);
}

size_t pmm_get_free(void) {
    return pmm.free_pages;
}
size_t pmm_get_total(void) {
    return pmm.total_pages;
}
size_t pmm_get_used(void) {
    return pmm.used_pages;
}
size_t pmm_get_usable(void) {
    return pmm.usable_pages;
}

static void pmm_set(uintptr_t phys) {
    uintptr_t page = PAGE_COUNT(phys);
    pmm.bitmap[page >> 5] |= 1u << (page & 31);
}
static void pmm_clear(uintptr_t phys) {
    uintptr_t page = PAGE_COUNT(phys);
    pmm.bitmap[page >> 5] &= ~(1u << (page & 31));
}
static int pmm_test(uintptr_t phys) {
    uintptr_t page = PAGE_COUNT(phys);
    return pmm.bitmap[page >> 5] & (1u << (page & 31));
}
static uintptr_t pmm_alloc_one(void) {
    /* Search for a free page 32 pages at a time. */

    if (pmm.free_pages == 0) {
        return 0;
    }

    if (pmm.hint >= pmm.total_pages) {
        pmm.hint = 0;
    }

    uintptr_t page = pmm.hint;
    size_t start = pmm.hint >> 5;

    /* Search from hint to end 
     * [start] 11111111 11111111 11111111 11100000 [end]
     *                                  ^ hint
     *                   search ------->              */
    for (size_t dword = start; dword < pmm.bitmap_size / sizeof(uint32_t); dword++) {
        assert(pmm.bitmap != NULL);

        uint32_t bits = pmm.bitmap[dword];

        /* Ignore pages before hint in the first dword */
        if (dword == start) {
            bits |= (1U << (page & 31)) - 1;
        }

        /* Skip dword if there are no allocations left */
        if (bits == 0xFFFFFFFF) {
            continue;
        }
        
        /* Find first zero bit */
        uint32_t free_bits = ~bits;
        uint32_t b = 0;

        while (!(free_bits & 1U)) {            
            free_bits >>= 1;
            b++;
        }

        page = (dword << 5) + b;

        if (page >= pmm.total_pages) {
            break;
        }

        pmm_set(page << 12);

        pmm.free_pages--;
        pmm.used_pages++;

        pmm.hint = page + 1;

        if (pmm.hint >= pmm.total_pages) {
            pmm.hint = 0;
        }

        return page << 12;
    }

    /* Search from start to hint 
     * [start] 11111111 11111111 11111111 11100000 [end]
     *                                  ^ hint
     *                                  <------ search */
    for (size_t dword = 0; dword <= start; dword++) {
        uint32_t bits = pmm.bitmap[dword];

        /* Ignore pages at and after hint in the final dword */
        if (dword == start) {
            bits |= ~((1U << (page & 31)) - 1);
        }

        /* Skip dword if there are no allocations left */
        if (bits == 0xFFFFFFFF) {
            continue;
        }

        /* Find first zero bit */
        uint32_t free_bits = ~bits;
        uint32_t b = 0;

        while (!(free_bits & 1U)) {
            free_bits >>= 1;
            b++;
        }

        page = (dword << 5) + b;

        if (page >= pmm.total_pages) {
            return 0;
        }

        pmm_set(page << 12);

        pmm.free_pages--;
        pmm.used_pages++;

        pmm.hint = page + 1;

        if (pmm.hint >= pmm.total_pages) {
            pmm.hint = 0;
        }

        return page << 12;
    }

    return 0;
}
static uintptr_t pmm_find_contiguous_pages(size_t count) {
    /* Find a contiguous range of physical pages */
    assert(pmm.bitmap != NULL);
    assert(pmm.bitmap_size != 0);

    uintptr_t start = 0;
    size_t run = 0;
    size_t len = pmm.bitmap_size / sizeof(uint32_t);

    if (count == 0) {
        return 0;
    }

    for (size_t dword = 0; dword < len; dword++) {
        uint32_t bits = pmm.bitmap[dword];

        for (uint32_t bit = 0; bit < 32U; bit++) {
            uintptr_t page = (dword << 5) + bit;

            if (page >= pmm.total_pages) {
                break;
            }

            if (bits & (1U << bit)) {
                run = 0;
                continue;
            }

            if (run == 0) {
                start = page;
            }

            run++;

            if (run == count) {
                return start;
            }
        }
    }
    return 0;
}
