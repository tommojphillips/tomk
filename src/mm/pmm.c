/* pmm.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * Physical address memory manager
 */

#include <stdint.h>
#include <string.h>

#include <pmm.h>
#include <kphysmap.h>
#include <kinit_alloc.h>
#include <align.h>
#include <paging.h>

#include <assert.h>
#include <kdprint.h>
#include <kernel.h>

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
    size_t largest_run;
    uintptr_t memory_end;
} pmm_t;

/* Physical Memory Manager */
static pmm_t pmm;

static void pmm_set(uintptr_t phys);
static void pmm_clear(uintptr_t phys);
static int pmm_test(uintptr_t phys);
static uintptr_t pmm_alloc_one(void);
static uintptr_t pmm_find_contiguous_pages(size_t count);

void pmm_init(const kphysmap_t* map) {
    assert(map != NULL);
    assert(map->count > 0);
    
    /* Find the highest usable physical address */
    pmm.memory_end = 0;
    pmm.usable_pages = 0;
    for (size_t i = 0; i < map->count; ++i) {
        if (!(map->regions[i].flags & KPHYSREGION_VALID)) {
            continue;
        }
        if ((map->regions[i].flags & KPHYSREGION_TYPE_MASK) != KPHYSREGION_TYPE_RAM) {
            continue;
        }
        
        uintptr_t end = map->regions[i].address + map->regions[i].size;

        if (end > pmm.memory_end) {
            pmm.memory_end = end;
        }

        uintptr_t start_page = ALIGN(uintptr_t, map->regions[i].address, PAGE_SIZE);
        uintptr_t end_page = end & ~(uintptr_t)(PAGE_SIZE - 1);

        if (end_page > start_page) {
            pmm.usable_pages += TO_PAGE(end_page - start_page);
        }
    }
    
    pmm.total_pages = TO_PAGE(pmm.memory_end + (PAGE_SIZE-1));
    pmm.bitmap_size = CEIL_DIV(pmm.total_pages, 32) * sizeof(uint32_t);
    pmm.free_pages = 0;
    pmm.used_pages = pmm.usable_pages;
    pmm.hint = 0;
    
    /* Allocate memory for the bitmap */
    pmm.bitmap = kinit_alloc(pmm.bitmap_size);
    assert(pmm.bitmap != NULL);

    /* Mark all physical addresses used */
    memset(pmm.bitmap, 0xFF, pmm.bitmap_size);
    
    /* Find all usable RAM regions in map and mark those pages free in PMM */
    for (size_t i = 0; i < map->count; ++i) {
        if (!(map->regions[i].flags & KPHYSREGION_VALID)) {
            continue;
        }
        if ((map->regions[i].flags & KPHYSREGION_TYPE_MASK) != KPHYSREGION_TYPE_RAM) {
            continue;
        }
        
        pmm_mark_free(map->regions[i].address, map->regions[i].size);
    }

    /* Mark zero page used */
    pmm_mark_used(0x00000000, 0x1000);
}

uintptr_t pmm_alloc(size_t count) {
    /* Allocate contiguous physical addresses */

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
        kprint("[PMM] Fragmentation error: %u (avail=%u largest_run=%u)\n", count, pmm.free_pages, pmm.largest_run);
        return 0;
    }

    /* Mark pages used */
    for (size_t i = 0; i < count; i++) {
        pmm_set(TO_ADDR(start + i));
    }

    pmm.free_pages -= count;
    pmm.used_pages += count;

    return TO_ADDR(start);
}
void pmm_free(uintptr_t phys, size_t count) {
    /* Free contiguous physical addresses */
    
    if (count == 0) {
        return;
    }
    
    /* Physical address must be page aligned */
    if (phys & (PAGE_SIZE - 1)) {
        kprint("[PMM] Error mis-aligned page: %8.8X\n", phys);
        return;
    }
    
    uintptr_t page = TO_PAGE(phys);

    /* Page must be managed by PMM */
    if (page >= pmm.total_pages || count > pmm.total_pages - page) {
        kprint("[PMM] Error address out of bounds: %8.8X\n", phys);
        return;
    }

    /* Mark pages free */
    for (size_t i = 0; i < count; i++) {
        pmm_clear(phys + TO_PAGE(i));
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
        uintptr_t page = TO_PAGE(p);
        
        if (page >= pmm.total_pages) {
            continue;
        }

        if (pmm_test(p)) {
            pmm_clear(p);
            pmm.free_pages++;
            pmm.used_pages--;
        }
    }

    kdprint("[PMM] Mark free: %08X-%08X\n", start, end);
}
void pmm_mark_used(uintptr_t phys, size_t size) {    
    if (phys > pmm.memory_end) {
        return;
    }

    uintptr_t start = phys & ~(PAGE_SIZE-1);
    uintptr_t end = (phys + size + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);

    for (uintptr_t p = start; p < end; p += PAGE_SIZE) {
        uintptr_t page = TO_PAGE(p);

        if (page >= pmm.total_pages) {
            continue;
        }

        if (!pmm_test(p)) {
            pmm_set(p);
            pmm.free_pages--;
            pmm.used_pages++;
        }
    }

    kdprint("[PMM] Mark used: %08X-%08X\n", start, end);
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
size_t pmm_get_largest_run(void) {
    return pmm.largest_run;
}

static void pmm_set(uintptr_t phys) {
    uintptr_t page = TO_PAGE(phys);
    pmm.bitmap[page >> 5] |= 1u << (page & 31);
}
static void pmm_clear(uintptr_t phys) {
    uintptr_t page = TO_PAGE(phys);
    pmm.bitmap[page >> 5] &= ~(1u << (page & 31));
}
static int pmm_test(uintptr_t phys) {
    uintptr_t page = TO_PAGE(phys);
    return pmm.bitmap[page >> 5] & (1u << (page & 31));
}
static uintptr_t pmm_alloc_one(void) {
    /* Search for a free page 32 pages at a time */

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

        pmm_set(TO_ADDR(page));

        pmm.free_pages--;
        pmm.used_pages++;

        pmm.hint = page + 1;

        if (pmm.hint >= pmm.total_pages) {
            pmm.hint = 0;
        }

        return TO_ADDR(page);
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

        pmm_set(TO_ADDR(page));

        pmm.free_pages--;
        pmm.used_pages++;

        pmm.hint = page + 1;

        if (pmm.hint >= pmm.total_pages) {
            pmm.hint = 0;
        }

        return TO_ADDR(page);
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
    
    pmm.largest_run = 0;

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
                if (pmm.largest_run < run) {
                    pmm.largest_run = run;
                }
                run = 0;
                continue;
            }

            if (run == 0) {
                start = page;
            }

            run++;

            if (run == count) {
                pmm.largest_run = run;
                return start;
            }
        }
    }
    return 0;
}
