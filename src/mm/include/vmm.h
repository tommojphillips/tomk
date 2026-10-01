/* vmm.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * Virtual address memory manager
 */

#ifndef _VMM_H
#define _VMM_H

#include <stdint.h>
#include <stddef.h>

typedef void* (*vmm_alloc_fn_t)(size_t);
typedef void (*vmm_free_fn_t)(void*);

typedef struct vmm_t {
    uint32_t* bitmap;
    size_t bitmap_size;
    size_t total_pages;
    size_t usable_pages;
    size_t free_pages;
    size_t used_pages;
    size_t largest_run;
    uintptr_t base;
    uintptr_t end;
} vmm_t;

/* Initialize vmm 
 base: virtual address base  
 end: virtual address end */
void vmm_init(vmm_t* vmm, uintptr_t base, uintptr_t end, vmm_alloc_fn_t alloc);

/* Destroy vmm */
void vmm_destroy(vmm_t* vmm, vmm_free_fn_t free);

/* Allocate virtual pages backed by arbitrary physical pages
 count: requested page count 
 Returns: pointer if successfull, otherwise NULL */
void* vmm_alloc(vmm_t* vmm, size_t count);

/* Allocate virtual pages backed by contiguous physical pages
 count: requested page count 
 Returns: pointer if successfull, otherwise NULL */
void* vmm_alloc_contiguous(vmm_t* vmm, size_t count);

/* Reserve non-backed, virtual pages
 count: requested page count 
 Returns: pointer if successfull, otherwise NULL */
void* vmm_reserve(vmm_t* vmm, size_t count);

/* Free non-backed virtual pages
 virt: Virtual page frame
 count: requested page count */
void vmm_unreserve(vmm_t* vmm, void* virt, size_t count);

/* Free backed virtual pages
 virt: Virtual page frame
 count: requested page count */
void vmm_free(vmm_t* vmm, void* virt, size_t count);

/* Get free virtual pages
 Returns: free virtual pages */
size_t vmm_get_free(vmm_t* vmm);

/* Get used virtual pages
 Returns: used virtual pages */
size_t vmm_get_used(vmm_t* vmm);

/* Get total virtual pages
 Returns: total virtual pages, including reserved virtual pages */
size_t vmm_get_total(vmm_t* vmm);

/* Get usable virtual pages
 Returns: usable virtual pages, excluding reserved virtual pages */
size_t vmm_get_usable(vmm_t* vmm);

/* Get previous largest run from vmm_alloc(), vmm_reserve(), vmm_alloc_contiguous()
 Returns: the largest run of virtual pages found  */
size_t vmm_get_largest_run(vmm_t* vmm);

/* Mark virtual page(s) free
 virt: Virtual page frame */
int vmm_mark_free(vmm_t* vmm, uintptr_t virt, size_t count);

/* Mark virtual page(s) used
 virt: Virtual page frame */
int vmm_mark_used(vmm_t* vmm, uintptr_t virt, size_t count);

#endif
