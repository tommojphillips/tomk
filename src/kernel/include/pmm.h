/* pmm.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * Physical address memory manager
 */

#ifndef _PMM_H
#define _PMM_H

#include <stdint.h>
#include <stddef.h>

/* Kernel Memory Map */
typedef struct kmmap_t kmmap_t;

/* Initialize pmm 
 kmmap: The kmmap */
void pmm_init(const kmmap_t* kmmap);

/* Allocate contiguous pages
 count: requested page count
 Returns: physical page address if successfull, otherwise 0. */
uintptr_t pmm_alloc(size_t count);

/* Free contiguous pages
 phys: physical page address
 count: allocated page count */
void pmm_free(uintptr_t phys, size_t count);

/* Get free pages
 Returns: free physical pages */
size_t pmm_get_free(void);

/* Get used pages
 Returns: used physical pages */
size_t pmm_get_used(void);

/* Get total pages
 Returns: total physical pages, including reserved physical pages */
size_t pmm_get_total(void);

/* Get usable pages
 Returns: usable physical pages, excluding reserved physical pages  */
size_t pmm_get_usable(void);

/* Mark physical page(s) free
 phys: physical page address
 size: region size (in bytes) */
void pmm_mark_free(uintptr_t phys, size_t size);

/* Mark physical page(s) used
 phys: physical page address
 size: region size (in bytes) */
void pmm_mark_used(uintptr_t phys, size_t size);

#endif
