/* vmm.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * Virtual address memory manager
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <vmm.h>
#include <pmm.h>
#include <paging.h>
#include <kalloc.h>
#include <align.h>

#include <assert.h>
#include <kdprint.h>
#include <kernel.h>

/* CEIL DIV */
#define CEIL_DIV(x,y) (((x) + (y) - 1) / (y))

static void vmm_set(vmm_t* vmm, uintptr_t virt);
static void vmm_clear(vmm_t* vmm, uintptr_t virt);
static int vmm_test(vmm_t* vmm, uintptr_t virt);
static void rollback_allocations(vmm_t* vmm, uintptr_t virt_addr, size_t count);
static uintptr_t vmm_find_contiguous_pages(vmm_t* vmm, size_t count);

void vmm_init(vmm_t* vmm, uintptr_t base, uintptr_t end, vmm_alloc_fn_t alloc) {
	assert((base & (PAGE_SIZE-1)) == 0);
	assert((end & (PAGE_SIZE-1)) == 0);
    assert(base < end);
    assert(alloc);
    assert(vmm);
    
    vmm->base = base;
    vmm->end = end;
    vmm->total_pages = TO_PAGE(end - base);
    vmm->bitmap_size = CEIL_DIV(vmm->total_pages, 32) * sizeof(uint32_t);
    vmm->free_pages = vmm->total_pages;
    vmm->usable_pages = vmm->total_pages;
    vmm->used_pages = 0;
    
    /* Allocate memory for the bitmap */
    vmm->bitmap = alloc(vmm->bitmap_size);
    assert(vmm->bitmap != NULL);

    /* Mark all virtual addresses free */
    memset(vmm->bitmap, 0, vmm->bitmap_size);

    if (base == 0) {
        /* Mark zero page used */
        vmm_mark_used(vmm, 0x00000000, 0x1000);
    }
}
void vmm_destroy(vmm_t* vmm, vmm_free_fn_t free) {
    assert(vmm != NULL);
    assert(vmm->bitmap != NULL);
    assert(free != NULL);

    /* Unmap and free each physical page independently */
    size_t len = vmm->bitmap_size / sizeof(uint32_t);
    for (size_t i = 0; i < len; i++) {
        uint32_t bits = vmm->bitmap[i];

        for (size_t bit = 0; bit < 32U; bit++) {
            size_t page = (i << 5) + bit;

            if (page >= vmm->total_pages) {
                break;
            }

            if (!(bits & (1U << bit))) {
                continue;
            }

            uintptr_t virt_addr = vmm->base + TO_ADDR(page);
            uintptr_t phys_addr = pg_virt2phys(virt_addr);
            if (phys_addr != 0) {                
                pg_unmap(virt_addr, 1);
                pmm_free(phys_addr, 1);    
            }

            vmm->bitmap[i] &= ~(1U << bit);
        }
    }

    free(vmm->bitmap);
    vmm->bitmap = NULL;
    vmm->bitmap_size = 0;
}

void* vmm_reserve(vmm_t* vmm, size_t count) {
    /* Non-backed contiguous virtual addresses */
    assert(vmm != NULL);

    if (count == 0) {
        return NULL;
    }

    if (count > vmm->free_pages) {
        return NULL;
    }

    /* Find a contiguous range of virtual pages */
    uintptr_t start = vmm_find_contiguous_pages(vmm, count);
    if (start == 0) {
        kprint("[VMM] Memory fragmentation error: %u (avail=%u largest_run=%u)\n", count, vmm->free_pages, vmm->largest_run);
        return NULL;
    }

    /* Reserve virtual addresses */
    for (size_t i = 0; i < count; i++) {
        uintptr_t virt_addr = start + TO_ADDR(i);
        vmm_set(vmm, virt_addr);
    }

    /* Bookkeeping */
    vmm->free_pages -= count;
    vmm->used_pages += count;
    
    return (void*)start;
}
void* vmm_alloc(vmm_t* vmm, size_t count) {
    /* Contiguous virtual addresses; Arbitrary physical addresses */
    assert(vmm != NULL);

    void* start = vmm_reserve(vmm, count);
    if (start == NULL) {
        return NULL;
    }

    /* Allocate and map each physical page independently */
    for (size_t i = 0; i < count; i++) {
        /* Allocate arbitrary physical page */
        uintptr_t phys = pmm_alloc(1);
        if (!phys) {
            rollback_allocations(vmm, (uintptr_t)start, i);
            return NULL;
        }
        
        uintptr_t virt = (uintptr_t)start + TO_ADDR(i);
        pg_map(virt, phys, PTE_RW, 1);
    }
    return start;
}
void* vmm_alloc_contiguous(vmm_t* vmm, size_t count) {
    /* Contiguous virtual addresses; Contiguous physical addresses */
    assert(vmm != NULL);

    void* virt = vmm_reserve(vmm, count);
    if (virt == NULL) {
        return NULL;
    }

    /* Allocate contiguous range of physical pages */
    uintptr_t phys = pmm_alloc(count);
    if (!phys) {
        return NULL;
    }
    
    /* Map all the addresses in one pass */
    pg_map((uintptr_t)virt, phys, PTE_RW, count);

    return virt;
}

int vmm_unreserve(vmm_t* vmm, void* virt, size_t count) {
    /* Contiguous virtual addresses; Arbitrary physical addresses */
    assert(vmm != NULL);

    if (virt == NULL) {
        return 1;
    }

    if (count == 0) {
        return 1;
    }

    if ((uintptr_t)virt < vmm->base || (uintptr_t)virt >= vmm->end) {
        kprint("[VMM] Out of bounds error: %8.8X\n", (uintptr_t)virt);
        return 0;
    }

    if ((uintptr_t)virt & (PAGE_SIZE - 1)) {
        kprint("[VMM] Mis-aligned page error: %8.8X\n", (uintptr_t)virt);
        return 0;
    }

    /* Unreserve pages */
    for (size_t i = 0; i < count; i++) {
        uintptr_t virt_addr = (uintptr_t)virt + TO_ADDR(i);
        vmm_clear(vmm, virt_addr);
    }

    /* Bookkeeping */
    vmm->free_pages += count;
    vmm->used_pages -= count;

    return 1;
}
void vmm_free(vmm_t* vmm, void* virt, size_t count) {
    /* Contiguous virtual addresses; Arbitrary physical addresses */

    /* Unreserve all virtual addresses */
    vmm_unreserve(vmm, virt, count);
    
    /* Unmap all virtual addresses */
    pg_unmap((uintptr_t)virt, count);
    
    /* Free each physical page independently */
    for (size_t i = 0; i < count; i++) {
        uintptr_t virt_addr = (uintptr_t)virt + TO_ADDR(i);        
        uintptr_t phys_addr = pg_virt2phys(virt_addr);
        if (phys_addr != 0) {
            pmm_free(phys_addr, 1);
        }
    }
}

int vmm_mark_free(vmm_t* vmm, uintptr_t virt, size_t size) {
    assert(virt >= vmm->base);
    assert(size <= vmm->end - virt);

    int r = 1;
    uintptr_t start = ALIGN(uintptr_t, virt, PAGE_SIZE);
    uintptr_t end = (virt + size) & ~(uintptr_t)(PAGE_SIZE - 1);

    for (uintptr_t virt_addr = start; virt_addr < end; virt_addr += PAGE_SIZE) {
        if (!vmm_test(vmm, virt_addr)) {
            r = 0;
            continue;
        }

        vmm_clear(vmm, virt_addr);
        vmm->free_pages++;
        vmm->used_pages--;
    }

    kdprint("[VMM] Mark free: %08X-%08X r=%d\n", start, end, r);
    return r;
}
int vmm_mark_used(vmm_t* vmm, uintptr_t virt, size_t size) {
    assert(virt >= vmm->base);
    assert(size <= vmm->end - virt);

    int r = 1;
    uintptr_t start = ALIGN(uintptr_t, virt, PAGE_SIZE);
    uintptr_t end = ALIGN(uintptr_t, virt + size, PAGE_SIZE);

    for (uintptr_t virt_addr = start; virt_addr < end; virt_addr += PAGE_SIZE) {        
        if (vmm_test(vmm, virt_addr)) {
            r = 0;
            continue;
        }

        vmm_set(vmm, virt_addr);
        vmm->free_pages--;
        vmm->used_pages++;
    }

    kdprint("[VMM] Mark used: %08X-%08X r=%d\n", start, end, r);
    return r;
}

size_t vmm_get_free(vmm_t* vmm) {
    return vmm->free_pages;
}
size_t vmm_get_total(vmm_t* vmm) {
    return vmm->total_pages;
}
size_t vmm_get_used(vmm_t* vmm) {
    return vmm->used_pages;
}
size_t vmm_get_usable(vmm_t* vmm) {
    return vmm->usable_pages;
}
size_t vmm_get_largest_run(vmm_t* vmm) {
    return vmm->largest_run;
}

static void vmm_set(vmm_t* vmm, uintptr_t virt) {
    uintptr_t page = TO_PAGE(virt - vmm->base);
    size_t i = page >> 5;
    if (i >= vmm->bitmap_size / sizeof(uint32_t)) {
        return;
    }
    vmm->bitmap[i] |= 1U << (page & 31);
}
static void vmm_clear(vmm_t* vmm, uintptr_t virt) {
    uintptr_t page = TO_PAGE(virt - vmm->base);
    size_t i = page >> 5;
    if (i >= vmm->bitmap_size / sizeof(uint32_t)) {
        return;
    }
    vmm->bitmap[i] &= ~(1U << (page & 31));
}
static int vmm_test(vmm_t* vmm, uintptr_t virt) {
    uintptr_t page = TO_PAGE(virt - vmm->base);
    size_t i = page >> 5;
    if (i >= vmm->bitmap_size / sizeof(uint32_t)) {
        return 0;
    }
    return vmm->bitmap[i] & (1U << (page & 31));
}
static void rollback_allocations(vmm_t* vmm, uintptr_t virt, size_t count) {
    for (size_t i = 0; i < count; i++) {
        uintptr_t virt_addr =  virt + TO_ADDR(i);
        uintptr_t phys_addr = pg_virt2phys(virt_addr);
        if (phys_addr != 0) {
            pg_unmap(virt_addr, 1);
            pmm_free(phys_addr, 1);
        }
        vmm_clear(vmm, virt_addr);
    }
}
static uintptr_t vmm_find_contiguous_pages(vmm_t* vmm, size_t count) {
    /* Find a contiguous range of virtual pages */
    assert(vmm != NULL);
    assert(vmm->bitmap != NULL);
    assert(vmm->bitmap_size != 0);

    uintptr_t start = 0;
    uint32_t run = 0;
    size_t len = vmm->bitmap_size / sizeof(uint32_t);
    
    vmm->largest_run = 0;

    if (count == 0) {
        return 0;
    }

    for (size_t i = 0; i < len; i++) {
        uint32_t bits = vmm->bitmap[i];

        for (size_t bit = 0; bit < 32U; bit++) {
            size_t page = (i << 5) + bit;

            if (page >= vmm->total_pages) {
                break;
            }

            if (bits & (1U << bit)) {
                if (vmm->largest_run < run) {
                    vmm->largest_run = run;
                }
                run = 0;
                continue;
            }

            if (run == 0) {
                start = page;
            }

            run++;

            if (run == count) {
                vmm->largest_run = run;
                return vmm->base + TO_ADDR(start);
            }
        }
    }
    return 0;
}
