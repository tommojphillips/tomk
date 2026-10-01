/* paging.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 * Implements Dynamic page directory/table allocation
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <pmm.h>
#include <vmm.h>
#include <paging.h>
#include <mman.h>

#include <kspacedef.h>
#include <assert.h>

uintptr_t pgd_create_pd(void) {
    
    /* Allocate a physical frame for the new page directory */
    uintptr_t pd_phys = pmm_alloc(1);
    if (pd_phys == 0) {
        return 0;
    }

    /* Map the new page directory */
    uint32_t* pd_virt = mmap(NULL, pd_phys, 0x1000, PTE_RW);
    if (pd_virt == NULL) {
        pmm_free(pd_phys, 1);
        return 0;
    }

    /* Map all kernel page tables into the new page directry */
    for (size_t i = PD_IDX(KVIRT); i < PD_IDX(KVIRT_END); i++) {
        pd_virt[i] = ((uint32_t*)0xFFFFF000)[i];
    }

    /* Map recursive PD */
    pd_virt[1023] = pd_phys | PTE_RW | PTE_P;

    /* Unmap temp page */
    munmap(pd_virt, 0x1000);

    return pd_phys;
}

void pgd_destroy_pd(uintptr_t pd) {
    pmm_free(pd, 1);
}

/*int pgd_map(uintptr_t virt, uintptr_t phys, uint32_t flags, size_t count) {

}

void pgd_unmap(uintptr_t virt, size_t count) {

}*/
