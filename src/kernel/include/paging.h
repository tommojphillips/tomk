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
#define PTE_RW 0x02 /* Read-Only/Read-Write*/
#define PTE_US 0x04 /* User/Super */
#define PTE_A  0x20 /* Accessed bit */
#define PTE_D  0x40 /* Dirty bit */

/* Page Table Entry struct */
typedef struct pte_t {
    union {
        uint32_t dword;
        struct {
            uint32_t present    : 1; /* Present */
            uint32_t rw         : 1; /* 0 = read only; 1 = read and write */
            uint32_t us         : 1; /* 0 = supervisor; 1 = user */
            uint32_t r3         : 1; 
            uint32_t r4         : 1;
            uint32_t accessed   : 1; /* Accessed */
            uint32_t dirty      : 1; /* 0 = page is unmodified; 1 = page is dirty (only valid in PTE) */
            uint32_t r7         : 1;
            uint32_t r8         : 1;
            uint32_t available  : 3; /* Available for systems programmer use */
            uint32_t page_frame : 20;
        };
    };
} pte_t;

/* Page Directory Entry struct */
typedef pte_t pde_t;

/* Locate PDE containing linear address.
 Returns a pointer to the page directory entry  */
extern pde_t* pg_loc_pde(uint32_t linear_address);

/* Locate PTE containing linear address.
 Returns a pointer to the page table entry */
extern pte_t* pg_loc_pte(uint32_t linear_address);

/* Map contiguous pages
linear_address:       linear start address
phyiscal_address:     physical start address
flags:                page flags
count:                page count */
extern void pg_map(uint32_t linear_address, uint32_t physical_address, uint32_t flags, uint32_t count);

/* Flush TLB 
Returns CR3 */
extern uint32_t pg_flush(void);

/* Flush TLB */
extern void pg_invalidate(uint32_t linear_address);

/* Enable paging 
Returns CR3 */
extern uint32_t pg_enable(void);

/* Disable paging 
Returns CR3 */
extern uint32_t pg_disable(void);

/* get physical address mapped to linear address.
Returns physical address
Returns 0 if not mapped */
extern uint32_t pg_get_physical(uint32_t linear_address);

#endif
