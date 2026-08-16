/* pmm.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * Physical address memory manager
 */

#ifndef _PMM_H
#define _PMM_H

#include <stdint.h>

/* Kernel Memory Map */
typedef struct kmmap_t kmmap_t;

/* Initialize pmm 
 kmmap: The kmmap */
void pmm_init(const kmmap_t* kmmap);

/* Allocate page */
uint32_t pmm_alloc(void);

/* Free page */
void pmm_free(uint32_t phys);

/* Get free pages */
uint32_t pmm_get_free_pages(void);

/* Get used pages */
uint32_t pmm_get_used_pages(void);

/* Get total pages */
uint32_t pmm_get_total_pages(void);

/* Get usable pages */
uint32_t pmm_get_usable_pages(void);

/* Mark page(s) free */
void pmm_mark_free(uint64_t phys, uint64_t size);

/* Mark page(s) used */
void pmm_mark_used(uint64_t phys, uint64_t size);

/* Mark page(s) reserved */
void pmm_mark_reserved(uint64_t phys, uint64_t size);

#endif
