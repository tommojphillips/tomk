/* pmm.c - Physical Memory Mananger */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <pmm.h>
#include <kmmap.h>
#include <kalloc.h>

#define KDBG
#ifdef KDBG
#define kprint(...) printf(__VA_ARGS__)
#else
#define kprint(...)
#endif

#define PAGE_SIZE 0x1000

typedef struct pmm_t {
    uint32_t* bitmap;
    uint32_t bitmap_size;

    uint32_t total_pages;
    uint32_t usable_pages;
    uint32_t free_pages;
    uint32_t used_pages;
} pmm_t;

/* Physical Memory Manager */
pmm_t pmm;

static void pmm_set(uint32_t phys) {
    pmm.bitmap[phys >> 5] |= 1u << (phys & 31);
}
static void pmm_clear(uint32_t phys) {
    pmm.bitmap[phys >> 5] &= ~(1u << (phys & 31));
}
static int pmm_test(uint32_t phys) {
    return pmm.bitmap[phys >> 5] & (1u << (phys & 31));
}

void pmm_mark_free(uint64_t phys, uint64_t size) {
    uint64_t start = (phys + PAGE_SIZE - 1) & ~(uint64_t)(PAGE_SIZE - 1);
    uint64_t end   = (phys + size) & ~(uint64_t)(PAGE_SIZE - 1);

    for (uint64_t p = start; p < end; p += PAGE_SIZE) {
        uint32_t page = (uint32_t)(p >> 12);
        
        if (page >= pmm.total_pages) {
            continue;
        }

        if (pmm_test(page)) {
            pmm_clear(page);
            pmm.free_pages++;
            pmm.used_pages--;
        }
    }
}
void pmm_mark_used(uint64_t phys, uint64_t size) {
    uint64_t start = phys & ~(uint64_t)(PAGE_SIZE - 1);
    uint64_t end = (phys + size + PAGE_SIZE - 1) & ~(uint64_t)(PAGE_SIZE - 1);

    for (uint64_t p = start; p < end; p += PAGE_SIZE) {
        uint32_t page = (uint32_t)(p >> 12);

        if (page >= pmm.total_pages) {
            continue;
        }

        if (!pmm_test(page)) {
            pmm_set(page);
            pmm.free_pages--;
            pmm.used_pages++;
        }
    }
}
void pmm_mark_reserved(uint64_t phys, uint64_t size) {
    uint64_t start = phys & ~(uint64_t)(PAGE_SIZE - 1);
    uint64_t end = (phys + size + PAGE_SIZE - 1) & ~(uint64_t)(PAGE_SIZE - 1);

    for (uint64_t p = start; p < end; p += PAGE_SIZE) {
        uint32_t page = (uint32_t)(p >> 12);

        if (page >= pmm.total_pages) {
            continue;
        }

        if (!pmm_test(page)) {
            pmm_set(page);
            pmm.free_pages--;
            pmm.usable_pages--;
        }
    }
}

void pmm_init(const kmmap_t* kmmap) {
    uint64_t memory_end = 0;

    /* Find the highest usable physical address */
    pmm.usable_pages = 0;
    for (size_t i = 0; i < kmmap->count; ++i) {
        if (!(kmmap->regions[i].flags & KMREGION_FLAG_VALID)) {
            continue;
        }
        if ((kmmap->regions[i].flags & KMREGION_FLAG_AR_MASK) != KMREGION_FLAG_AR_RW) {
            continue;
        }
        
        uint64_t end = kmmap->regions[i].address + kmmap->regions[i].size;

        if (end > memory_end) {
            memory_end = end;
        }

        uint64_t start_page = (kmmap->regions[i].address + PAGE_SIZE - 1) & ~(uint64_t)(PAGE_SIZE - 1);
        uint64_t end_page = end & ~(uint64_t)(PAGE_SIZE - 1);

        if (end_page > start_page) {
            pmm.usable_pages += (uint32_t)((end_page - start_page) >> 12);
        }
    }

    pmm.total_pages = (uint32_t)((memory_end + PAGE_SIZE - 1) >> 12);
    pmm.bitmap_size = ((pmm.total_pages + 31) >> 5) * sizeof(uint32_t);
    pmm.free_pages = 0;
    pmm.used_pages = pmm.usable_pages;
    pmm.bitmap = kalloc(pmm.bitmap_size);

    /* Mark all RAM used */
    memset(pmm.bitmap, 0xFF, pmm.bitmap_size);
    
    /* Mark usable RAM free */
    for (size_t i = 0; i < kmmap->count; ++i) {
        if (!(kmmap->regions[i].flags & KMREGION_FLAG_VALID)) {
            continue;
        }
        if ((kmmap->regions[i].flags & KMREGION_FLAG_AR_MASK) != KMREGION_FLAG_AR_RW) {
            continue;
        }
        
        pmm_mark_free(kmmap->regions[i].address, kmmap->regions[i].size);
    }
}

uint32_t pmm_alloc(void) {
    uint32_t bits;
    uint32_t bit;
    uint32_t dword;
    uint32_t page;
    uint32_t addr;
    for (dword = 0; dword < pmm.bitmap_size / sizeof(uint32_t); dword++) {
        bits = pmm.bitmap[dword];

        if (bits == 0xFFFFFFFF) {
            continue;
        }

        for (bit = 0; bit < 32U; bit++) {
            if (bits & (1U << bit)) {
                continue;
            }

            page = (dword << 5U) + bit;
            if (page >= pmm.total_pages) {
                kprint("[PMM] Error out of pages\n");
                return 0;
            }
            addr = page << 12;

            pmm_set(page);
            pmm.free_pages--;
            pmm.used_pages++;
            return addr;
        }
    }

    kprint("[PMM] Error out of pages\n");
    return 0;
}
void pmm_free(uint32_t address) {
    uint32_t page = address >> 12;

    /* Must be within limits */
    if (page >= pmm.total_pages) {
        kprint("[PMM] Error address out of bounds: %8.8X\n", address);
        return;
    }

    /* Must be page-aligned */
    if (address & (PAGE_SIZE - 1)) {
        kprint("[PMM] Error mis-aligned page: %8.8X\n", address);
        return;
    }

    /* Check for double free */
    if (!pmm_test(page)) {
        kprint("[PMM] Error double free page: %8.8X\n", address);
        return;
    }

    pmm_clear(page);
    pmm.free_pages++;
    pmm.used_pages--;
}

uint32_t pmm_get_free_pages(void) {
    return pmm.free_pages;
}
uint32_t pmm_get_total_pages(void) {
    return pmm.total_pages;
}
uint32_t pmm_get_used_pages(void) {
    return pmm.used_pages;
}
uint32_t pmm_get_usable_pages(void) {
    return pmm.usable_pages;
}
