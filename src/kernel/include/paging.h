/* kernel/include/paging.h */

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

/* Map single page (4096 bytes)
pd_base:              page directory address.
phyiscal_address:     physical address.
linear_address:       linear address.
flags:                page flags. */
extern void paging_map_page(uint32_t pd_base, uint32_t physical_address, uint32_t linear_address, uint32_t flags);

/* Map contiguous pages
pd_base:              page directory address.
phyiscal_address:     physical start address.
linear_address:       linear start address.
flags:                page flags.
end_physical_address: physical end address. (exclusive) (increments of 4096 bytes) */
extern void paging_map_pages(uint32_t pd_base, uint32_t physical_address, uint32_t linear_address, uint32_t flags, uint32_t end_physical_address);

/* Map contiguous pages
pd_base:              page directory address.
phyiscal_address:     physical start address.
linear_address:       linear start address.
flags:                page flags.
count:                count in bytes */
extern void paging_map_bytes(uint32_t pd_base, uint32_t physical_address, uint32_t linear_address, uint32_t flags, uint32_t count);

#endif
