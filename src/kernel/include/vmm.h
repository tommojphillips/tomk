/* vmm.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _VMM_H
#define _VMM_H

#include <stdint.h>

/* Initialize pmm */
void vmm_init(uint32_t base, uint32_t end);

/* Allocate page */
void* vmm_alloc(void);

/* Free page */
void vmm_free(void* virt);

/* Get free pages */
uint32_t vmm_get_free_pages(void);

/* Get used pages */
uint32_t vmm_get_used_pages(void);

/* Get total pages */
uint32_t vmm_get_total_pages(void);

/* Get usable pages */
uint32_t vmm_get_usable_pages(void);

#endif
