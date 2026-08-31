/* paging.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_SIZE     4096
#define PD_ENTRY_SIZE 4
#define PD_ENTRIES    1024
#define PD_SIZE       (PD_ENTRY_SIZE * PD_ENTRIES)
#define PT_ENTRIES    1024
#define PT_SIZE       (PAGE_SIZE * PT_ENTRIES)

#define PTE_NP 0x00 /* Not Present */
#define PTE_P  0x01 /* Present */
#define PTE_RO 0x00 /* Read-Only */
#define PTE_RW 0x02 /* Read-Write*/
#define PTE_US 0x04 /* User/Super */

/* Map contiguous pages
virtual_address:  virtual start address
phyiscal_address: physical start address
flags:            page flags
count:            page count */
extern void pg_map(uint32_t virtual_address, uint32_t physical_address, uint32_t flags, uint32_t count);

/* Unmap contiguous pages
virtual_address:  virtual start address
count:            page count */
extern void pg_unmap(uint32_t virtual_address, uint32_t count);

/* Flush TLB */
extern void pg_flush(void);

/* Invalidate page table entry in TLB
 virtual_address: start virtual address
 count:           page count */
extern void pg_invalidate(uint32_t virtual_address, uint32_t count);

/* Get the physical address that is mapped to virtual address
 virtual_address: the virtual address to convert
 returns 0 if the physical address isnt mapped, otherwise returns the physical address */
extern uint32_t pg_virt2phys(uint32_t virtual_address);

/* Change virtual address change access permissions (RW/US bits) */
extern void pg_chgpriv(uint32_t virtual_address, uint32_t flags, uint32_t count);

#endif
