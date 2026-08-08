/* paging.h */

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
#define PTE_RW 0x02 /* Read-Only/Read-Write*/
#define PTE_US 0x04 /* User/Super */
#define PTE_A  0x20 /* Accessed bit */
#define PTE_D  0x40 /* Dirty bit */

/* Map contiguous pages
pd_base:              page directory address
linear_address:       linear start address
phyiscal_address:     physical start address
flags:                page flags
count:                page count */
extern void paging_map(uint32_t pd_base, uint32_t linear_address, uint32_t physical_address, uint32_t flags, uint32_t count);

/* Flush TLB 
Returns CR3 */
extern uint32_t paging_flush(void);

/* Enable paging 
Returns CR3 */
extern uint32_t paging_enable(void);

/* Disable paging 
Returns CR3 */
extern uint32_t paging_disable(void);

#endif
