/* vmm.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <pmm.h>
#include <paging.h>
#include <kalloc.h>

#define KDBG
#ifdef KDBG
#define kprint(...) printf(__VA_ARGS__)
#else
#define kprint(...)
#endif

typedef struct vmm_t {
    uint32_t* bitmap;
    uint32_t bitmap_size;

    uint32_t base;
    uint32_t end;

    uint32_t total_pages;
    uint32_t usable_pages;
    uint32_t free_pages;
    uint32_t used_pages;
} vmm_t;

/* Virtual Memory Manager */
vmm_t vmm;

static void vmm_set(uint32_t virt) {
    vmm.bitmap[virt >> 5] |= 1u << (virt & 31);
}
static void vmm_clear(uint32_t virt) {
    vmm.bitmap[virt >> 5] &= ~(1u << (virt & 31));
}
static int vmm_test(uint32_t virt) {
    return vmm.bitmap[virt >> 5] & (1u << (virt & 31));
}

void vmm_mark_free(uint64_t virt, uint64_t size) {
    uint64_t start = (virt + PAGE_SIZE - 1) & ~(uint64_t)(PAGE_SIZE - 1);
    uint64_t end   = (virt + size) & ~(uint64_t)(PAGE_SIZE - 1);

    for (uint64_t p = start; p < end; p += PAGE_SIZE) {
        uint32_t page = (uint32_t)(p >> 12);
        
        if (page >= vmm.total_pages) {
            continue;
        }

        if (vmm_test(page)) {
            vmm_clear(page);
            vmm.free_pages++;
            vmm.used_pages--;
        }
    }
}
void vmm_mark_used(uint64_t virt, uint64_t size) {
    uint64_t start = virt & ~(uint64_t)(PAGE_SIZE - 1);
    uint64_t end = (virt + size + PAGE_SIZE - 1) & ~(uint64_t)(PAGE_SIZE - 1);

    for (uint64_t p = start; p < end; p += PAGE_SIZE) {
        uint32_t page = (uint32_t)(p >> 12);

        if (page >= vmm.total_pages) {
            continue;
        }

        if (!vmm_test(page)) {
            vmm_set(page);
            vmm.free_pages--;
            vmm.used_pages++;
        }
    }
}
void vmm_mark_reserved(uint64_t virt, uint64_t size) {
    uint64_t start = virt & ~(uint64_t)(PAGE_SIZE - 1);
    uint64_t end = (virt + size + PAGE_SIZE - 1) & ~(uint64_t)(PAGE_SIZE - 1);

    for (uint64_t p = start; p < end; p += PAGE_SIZE) {
        uint32_t page = (uint32_t)(p >> 12);

        if (page >= vmm.total_pages) {
            continue;
        }

        if (!vmm_test(page)) {
            vmm_set(page);
            vmm.free_pages--;
            vmm.usable_pages--;
        }
    }
}

void vmm_init(uint32_t base, uint32_t end) {
    vmm.base = base;
    vmm.end = end;
    vmm.total_pages = (end - base) >> 12;
    vmm.bitmap_size = (vmm.total_pages + 31) / 32 * sizeof(uint32_t);
    vmm.free_pages = vmm.total_pages;
    vmm.usable_pages = vmm.total_pages;
    vmm.used_pages = 0;
    vmm.bitmap = kalloc(vmm.bitmap_size);

    /* Mark all RAM free */
    memset(vmm.bitmap, 0, vmm.bitmap_size);

    /* Mark first page reserved */
    vmm_mark_reserved(0x00000000, 0x1000);
}

void* vmm_alloc(void) {
    uint32_t dword;
    uint32_t bits;
    uint32_t bit;
    uint32_t page;
    uint32_t phys_addr;
    uint32_t virt_addr;

    for (dword = 0; dword < vmm.bitmap_size / sizeof(uint32_t); dword++) {        
        bits = vmm.bitmap[dword];
        if (bits == 0xFFFFFFFF) {
            continue;
        }

        for (bit = 0; bit < 32; bit++) {
            if (bits & (1u << bit)) {
                continue;
            }

            page = (dword << 5) + bit;
            if (page >= vmm.total_pages) {
                return NULL;
            }

            phys_addr = pmm_alloc();
            if (!phys_addr) {
                return NULL;
            }

            virt_addr = vmm.base + page * PAGE_SIZE;

            vmm_set(page);
            vmm.free_pages--;
            vmm.used_pages++;

            paging_map(virt_addr, phys_addr, PTE_P | PTE_RW, 1);
            paging_invalidate(virt_addr);

            return (void*)virt_addr;
        }
    }

    return NULL;
}
void vmm_free(void* virt) {
    uint32_t virt_addr;
    uint32_t page;
    uint32_t phys;
    
    if (virt == NULL) {
        return;
    }
    
    virt_addr = (uint32_t)virt;

    /* Must be within limits */
    if (virt_addr < vmm.base || virt_addr >= vmm.end) {
        kprint("[VMM] Error address out of bounds: %8.8X\n", virt_addr);
        return;
    }

    /* Must be page-aligned */
    if (virt_addr & (PAGE_SIZE - 1)) {
        kprint("[VMM] Error mis-aligned page: %8.8X\n", virt_addr);
        return;
    }

    page = (virt_addr - vmm.base) >> 12;

    /* Check for double free */
    if (!vmm_test(page)) {
        kprint("[VMM] Error double free page: %8.8X\n", virt_addr);
        return;
    }

    phys = paging_get_physical(virt_addr);
    if (!phys) {
        kprint("[VMM] Error page not mapped: %8.8X\n", virt_addr);
        return;
    }

    paging_map(virt_addr, phys, PTE_NP, 1);
    paging_invalidate(virt_addr);
    pmm_free(phys);
    vmm_clear(page);
    vmm.free_pages++;
    vmm.used_pages--;
}

uint32_t vmm_get_free_pages(void) {
    return vmm.free_pages;
}
uint32_t vmm_get_total_pages(void) {
    return vmm.total_pages;
}
uint32_t vmm_get_used_pages(void) {
    return vmm.used_pages;
}
uint32_t vmm_get_usable_pages(void) {
    return vmm.usable_pages;
}
