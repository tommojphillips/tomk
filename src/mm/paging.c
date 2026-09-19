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
#include <kspacedef.h>
#include <assert.h>

uintptr_t pgd_create_pd(void) {
    extern vmm_t kvmm;
    
    /* Allocate a physical frame for the new page directory */
    uintptr_t pd_phys = pmm_alloc(1);
    if (pd_phys == 0) {
        return 0;
    }

    /* Reserve a page in the vmm for setting up the new page 
     directory, map the new pd physical address */
    uint32_t* pd_virt = vmm_reserve(&kvmm, 1);
    if (pd_virt == 0) {
        pmm_free(pd_phys, 1);
        return 0;
    }
    pg_map((uintptr_t)pd_virt, pd_phys, PTE_RW, 1);

    /* Map all kernel page tables into the new page directry */
    for (size_t i = PD_IDX(KVIRT); i < PD_IDX(KVIRT_END); i++) {
        pd_virt[i] = ((uint32_t*)0xFFFFF000)[i];
    }

    /* Map recursive PD */
    pd_virt[1023] = pd_phys | PTE_RW | PTE_P;

    /* free/unmap temp page */
    pg_unmap((uintptr_t)pd_virt, 1);
    vmm_unreserve(&kvmm, pd_virt, 1);

    return pd_phys;
}

void pgd_destroy_pd(uintptr_t pd) {
    pmm_free(pd, 1);
}

/*int pgd_map(uintptr_t virt, uintptr_t phys, uint32_t flags, size_t count) {

}

void pgd_unmap(uintptr_t virt, size_t count) {

}*/
