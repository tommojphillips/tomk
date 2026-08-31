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
#include <kernel.h>
#include <assert.h>
#include <align.h>

/* CEIL DIV */
#define CEIL_DIV(x,y) (((x) + (y) - 1) / (y))

typedef struct vmm_t {
    uint32_t* bitmap;
    size_t bitmap_size;

    uint32_t base;
    uint32_t end;

    uint32_t total_pages;
    uint32_t usable_pages;
    uint32_t free_pages;
    uint32_t used_pages;
} vmm_t;

/* Virtual Memory Manager */
static vmm_t vmm;

static void vmm_set(uint32_t virt);
static void vmm_clear(uint32_t virt);
static int vmm_test(uint32_t virt);
static void rollback_allocations(uint32_t virt_addr, size_t count);
static uint32_t vmm_find_contiguous_pages(size_t count);

void vmm_init(uint32_t base, uint32_t end) {
	assert((base & 0x00000FFF) == 0);
	assert((end & 0x00000FFF) == 0);
    assert(base < end);
    
    vmm.base = base;
    vmm.end = end;
    vmm.total_pages = (end - base) >> 12;
    vmm.bitmap_size = CEIL_DIV(vmm.total_pages, 32) * sizeof(uint32_t);
    vmm.free_pages = vmm.total_pages;
    vmm.usable_pages = vmm.total_pages;
    vmm.used_pages = 0;    
    vmm.bitmap = kalloc(vmm.bitmap_size);

    assert(vmm.bitmap != NULL);

    /* mark all virtual addresses free */
    memset(vmm.bitmap, 0, vmm.bitmap_size);

    if (base == 0) {
        /* mark zero page used */
        vmm_mark_used(0x00000000, 0x1000);
    }
}

void* vmm_alloc(size_t count) {
    /* Contiguous virtual addresses; Arbitrary physical addresses */

    if (count == 0) {
        return NULL;
    }

    if (count > vmm.free_pages) {
        return NULL;
    }

    /* Find a contiguous range of virtual pages. */
    uint32_t start = vmm_find_contiguous_pages(count);
    if (start == 0) {        
        kprint("[VMM] fragmentation error: %u\n", count);
        return NULL;
    }

    /* Allocate and map each physical page independently.
     Physical pages are not required to be contiguous, but they can be. */
    for (size_t i = 0; i < count; i++) {
        uint32_t phys_addr = pmm_alloc(1);
        uint32_t virt_addr = start + i * PAGE_SIZE;

        if (!phys_addr) {
            rollback_allocations(start, i);
            return NULL;
        }

        vmm_set(virt_addr);
        assert(vmm_test(virt_addr));
        pg_map(virt_addr, phys_addr, PTE_RW, 1);

    }

    kdprint("[VMM] alloc: virt_addr=%08X phys_addr=%08X count=%d\n", start, pg_virt2phys(start), count);

    /* Bookkeeping */
    vmm.free_pages -= count;
    vmm.used_pages += count;
    
    return (void*)start;
}
void vmm_free(void* virt, size_t count) {
    /* Contiguous virtual addresses; Arbitrary physical addresses */

    if (virt == NULL) {
        return;
    }

    if (count == 0) {
        return;
    }

    if ((uint32_t)virt < vmm.base || (uint32_t)virt >= vmm.end) {
        kprint("[VMM] error address out of bounds: %8.8X\n", (uint32_t)virt);
        return;
    }

    if ((uint32_t)virt & (PAGE_SIZE - 1)) {
        kprint("[VMM] error mis-aligned page: %8.8X\n", (uint32_t)virt);
        return;
    }

    uint32_t page = ((uint32_t)virt - vmm.base) >> 12;

    if (count > vmm.total_pages - page) {
        kprint("[VMM] error range out of bounds: %8.8X\n", (uint32_t)virt);
        return;
    }

    /* Validate the entire range */
    for (size_t i = 0; i < count; i++) {
        uint32_t virt_addr = (uint32_t)virt + i * PAGE_SIZE;
        if (!vmm_test(virt_addr)) {
            kprint("[VMM] error page not allocated: %8.8X\n", virt_addr);
            return;
        }

        uint32_t phys_addr = pg_virt2phys(virt_addr);
        if (!phys_addr) {
            kprint("[VMM] error page not mapped: %8.8X\n", virt_addr);
            return;
        }
    }

    /* Unmap and free each physical page independently */
    for (size_t i = 0; i < count; i++) {
        uint32_t virt_addr = (uint32_t)virt + i * PAGE_SIZE;
        vmm_clear(virt_addr);
        assert(!vmm_test(virt_addr));
        uint32_t phys_addr = pg_virt2phys(virt_addr);
        pmm_free(phys_addr, 1);
    }

    pg_unmap((uint32_t)virt, count);
    pg_invalidate((uint32_t)virt, count);

    /* Bookkeeping */
    vmm.free_pages += count;
    vmm.used_pages -= count;
}

void* vmm_alloc_contiguous(size_t count) {
    /* Contiguous virtual addresses; Contiguous physical addresses */
   
    if (count == 0) {
        return NULL;
    }

    if (count > vmm.free_pages) {
        kprint("[VMM] error out of pages\n");
        return NULL;
    }
    
    /* Find a contiguous range of virtual pages */
    uint32_t start = vmm_find_contiguous_pages(count);
    if (start == 0) {        
        kprint("[VMM] fragmentation error: %u\n", count);
        return NULL;
    }
    
    /* Allocate contiguous range of physical pages */
    uint32_t phys_addr = pmm_alloc(count);
    if (!phys_addr) {
        return NULL;
    }

    /* Mark the virtual range used and map the physical range. */
    for (uint32_t i = 0; i < count; i++) {
        uint32_t virt_addr = start + i * PAGE_SIZE;
        vmm_set(virt_addr);
        assert(vmm_test(virt_addr));
    }

    /* Since both virtual and physical address spaces are contiguous, map all the addresses in one pass */
    pg_map(start, phys_addr, PTE_RW, count);
    
    /* Bookkeeping */
    vmm.free_pages -= count;
    vmm.used_pages += count;

    return (void*)start;
}
void vmm_free_contiguous(void* virt, size_t count) {
    /* Contiguous virtual addresses -> Contiguous physical addresses */
    if (virt == NULL || count == 0) {
        return;
    }

    /* Must be within limits. */
    if ((uint32_t)virt < vmm.base || (uint32_t)virt >= vmm.end) {
        kprint("[VMM] error address out of bounds: %8.8X\n", (uint32_t)virt);
        return;
    }

    /* Must be page-aligned. */
    if ((uint32_t)virt & (PAGE_SIZE - 1)) {
        kprint("[VMM] error mis-aligned page: %8.8X\n", (uint32_t)virt);
        return;
    }

    uint32_t page = ((uint32_t)virt - vmm.base) >> 12;

    /* Make sure the entire virtual range is allocated. */
    if (count > vmm.total_pages - page) {
        kprint("[VMM] error range out of bounds: %8.8X\n", (uint32_t)virt);
        return;
    }

    for (size_t i = 0; i < count; i++) {
        uint32_t virt_addr = (uint32_t)virt + i * PAGE_SIZE;
        if (!vmm_test(virt_addr)) {
            kprint("[VMM] error unallocated page: %8.8X\n", virt_addr);
            return;
        }
    }

    /* Get the physical base before unmapping. */
    uint32_t phys_addr = pg_virt2phys((uint32_t)virt);

    if (!phys_addr) {
        kprint("[VMM] error page not mapped: %8.8X\n", (uint32_t)virt);
        return;
    }

    /* Unmap the entire virtual range */
    for (size_t i = 0; i < count; i++) {
        uint32_t virt_addr = (uint32_t)virt + i * PAGE_SIZE;
        vmm_clear(virt_addr);
        assert(!vmm_test(virt_addr));
    }

    pg_unmap((uint32_t)virt, count);
    pg_invalidate((uint32_t)virt, count);

    /* Since vmm_alloc_contiguous() obtains physically contiguous
     pages, free the physical range as one contiguous allocation */
    pmm_free(phys_addr, count);

    /* Bookkeeping */
    vmm.free_pages += count;
    vmm.used_pages -= count;
}

void vmm_mark_free(uint32_t virt, size_t size) {
    assert(virt >= vmm.base);
    assert(size <= vmm.end - virt);

    /* Only 4096-byte regions can be marked using the bitmap. Compute the usable size.
     Since pages are being marked as free, round end down to the nearest page. */
    uint32_t start = ALIGN(uint32_t, virt, PAGE_SIZE);
    uint32_t end = (virt + size) & ~(uint32_t)(PAGE_SIZE - 1);

    for (uint32_t virt_addr = start; virt_addr < end; virt_addr += PAGE_SIZE) {
        if (vmm_test(virt_addr)) {
            vmm_clear(virt_addr);
            vmm.free_pages++;
            vmm.used_pages--;
        }
    }

    kdprint("[VMM] mark free: %08X-%08X\n", start, end);
}
void vmm_mark_used(uint32_t virt, size_t size) {
    assert(virt >= vmm.base);
    assert(size <= vmm.end - virt);

    /* Only 4096-byte regions can be marked using the bitmap. Compute the usable size.
     Since pages are being marked as used, round end up to the nearest page. */
    uint32_t start = ALIGN(uint32_t, virt, PAGE_SIZE);
    uint32_t end = ALIGN(uint32_t, virt + size, PAGE_SIZE);

    for (uint32_t virt_addr = start; virt_addr < end; virt_addr += PAGE_SIZE) {        
        if (!vmm_test(virt_addr)) {
            vmm_set(virt_addr);
            vmm.free_pages--;
            vmm.used_pages++;
        }
    }

    kdprint("[VMM] mark used: %08X-%08X\n", start, end);
}

uint32_t vmm_get_free(void) {
    return vmm.free_pages;
}
uint32_t vmm_get_total(void) {
    return vmm.total_pages;
}
uint32_t vmm_get_used(void) {
    return vmm.used_pages;
}
uint32_t vmm_get_usable(void) {
    return vmm.usable_pages;
}

static void vmm_set(uint32_t virt) {
    uint32_t page = (virt - vmm.base) >> 12;
    uint32_t i = page >> 5;
    if (i >= vmm.bitmap_size / sizeof(uint32_t)) {
        return;
    }
    vmm.bitmap[i] |= 1U << (page & 31);
}
static void vmm_clear(uint32_t virt) {
    uint32_t page = (virt - vmm.base) >> 12;
    uint32_t i = page >> 5;
    if (i >= vmm.bitmap_size / sizeof(uint32_t)) {
        return;
    }
    vmm.bitmap[i] &= ~(1U << (page & 31));
}
static int vmm_test(uint32_t virt) {
    uint32_t page = (virt - vmm.base) >> 12;
    uint32_t i = page >> 5;
    if (i >= vmm.bitmap_size / sizeof(uint32_t)) {
        return 0;
    }
    return vmm.bitmap[i] & (1U << (page & 31));
}
static void rollback_allocations(uint32_t virt, size_t count) {
    for (size_t i = 0; i < count; i++) {
        uint32_t virt_addr =  virt + i * PAGE_SIZE;
        uint32_t phys_addr = pg_virt2phys(virt_addr);

        pmm_free(phys_addr, 1);
        vmm_clear(virt_addr);
        pg_unmap(virt_addr, 1);
        pg_invalidate(virt_addr, 1);
    }
}
static uint32_t vmm_find_contiguous_pages(size_t count) {
    /* Find a contiguous range of virtual pages */
    assert(vmm.bitmap != NULL);
    assert(vmm.bitmap_size != 0);

    uint32_t start = 0;
    uint32_t run = 0;
    size_t len = vmm.bitmap_size / sizeof(uint32_t);
    
    if (count == 0) {
        return 0;
    }

    for (size_t i = 0; i < len; i++) {
        uint32_t bits = vmm.bitmap[i];

        for (size_t bit = 0; bit < 32U; bit++) {
            size_t page = (i << 5) + bit;

            if (page >= vmm.total_pages) {
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
                return vmm.base + start * PAGE_SIZE;
            }
        }
    }
    return 0;
}
