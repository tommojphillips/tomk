/* paging.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * x86-32 paging abstraction
 */

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE     4096
#define TO_PAGE(_addr) ((_addr) >> 12)
#define TO_ADDR(_page) ((_page) << 12)

/* Convert virtual address to PD index */
#define PD_IDX(_virt_addr) ((_virt_addr) >> 22)

/* Convert virtual address to PT index */
#define PT_IDX(_virt_addr) (((_virt_addr) >> 12) & 0x3FF)

#define PTE_NP          0x00 /* Not-Present */
#define PTE_P           0x01 /* Present */

#define PTE_RO          0x00 /* Read-Only */
#define PTE_RW          0x02 /* Read-Write */

#define PTE_US          0x04 /* User/Super */

/* Invalidate all pages */
extern void pg_flush(void);

/* Invalidate page(s)
 virt:  virtual start address
 count: page count */
extern void pg_invalidate(uintptr_t virt, size_t count);

/* paging-static: Map contiguous pages
virt:  virtual start address
phys:  physical start address
flags: page flags
count: page count */
extern void pg_map(uintptr_t virt, uintptr_t phys, uint32_t flags, size_t count);

/* paging-static: Unmap contiguous pages
virt:  virtual start address
count: page count */
extern void pg_unmap(uintptr_t virt, size_t count);

/* paging-static: Get the physical address that is mapped to virtual address
 virt: virtual address
 returns 0 if the physical address isnt mapped, otherwise returns the physical address */
extern uintptr_t pg_virt2phys(uintptr_t virt);

/* paging-static: Change virtual address change access permissions (RW/US bits) */
extern void pg_chgpriv(uintptr_t virt, uint32_t flags, size_t count);



#endif
