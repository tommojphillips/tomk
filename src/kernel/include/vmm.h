/* vmm.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * Virtual address memory manager
 */

#ifndef _VMM_H
#define _VMM_H

#include <stdint.h>
#include <stddef.h>

/* Initialize pmm 
 base: virtual address base  
 end: virtual address end */
void vmm_init(uint32_t base, uint32_t end);

/* Allocate contiguous virtual pages backed by arbitrary physical pages.
 count: requested page count 
 Returns: virtual page address if successfull, otherwise 0. */
void* vmm_alloc(size_t count);

/* Free contiguous virtual pages backed by arbitrary physical pages.
 virt: Virtual page address
 count: requested page count */
void vmm_free(void* virt, size_t count);

/* Allocate contiguous virtual pages backed by contiguous physical pages.
 count: requested page count 
 Returns: virtual page address if successfull, otherwise 0. */
void* vmm_alloc_contiguous(size_t count);

/* Free contiguous virtual pages backed by contiguous physical pages.
 virt: Virtual page address 
 count: requested page count */
void vmm_free_contiguous(void* virt, size_t count);

/* Get free virtual pages
 Returns: free virtual pages */
size_t vmm_get_free(void);

/* Get used virtual pages
 Returns: used virtual pages */
size_t vmm_get_used(void);

/* Get total virtual pages
 Returns: total virtual pages, including reserved virtual pages */
size_t vmm_get_total(void);

/* Get usable virtual pages
 Returns: usable virtual pages, excluding reserved virtual pages */
size_t vmm_get_usable(void);

/* Mark virtual page(s) free
 virt: Virtual page address */
void vmm_mark_free(uint32_t virt, size_t count);

/* Mark virtual page(s) used
 virt: Virtual page address */
void vmm_mark_used(uint32_t virt, size_t count);

#endif
