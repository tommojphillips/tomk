/* mman.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>

#include <vmm.h>
#include <paging.h>

void* mmap(void* virt_addr, uintptr_t phys_addr, size_t length, uint32_t flags) {
    (void)virt_addr;
    extern vmm_t kvmm;
    size_t count = TO_PAGE(length + PAGE_SIZE-1);
    void* virt = vmm_reserve(&kvmm, count);
    if (virt == NULL) {
        return NULL;
    }
    pg_map((uintptr_t)virt, phys_addr, flags, count);
    return virt;
}

void munmap(void* virt, size_t length) {
    extern vmm_t kvmm;
    size_t count = TO_PAGE(length + PAGE_SIZE-1);
    
    if (virt == NULL) {
        return;
    }

    pg_unmap((uintptr_t)virt, count);
    vmm_unreserve(&kvmm, virt, count);
}
