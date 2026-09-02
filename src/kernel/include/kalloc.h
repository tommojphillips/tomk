/* kinit_alloc.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 *  Primitive Allocator for early kernel initialization
 */

#ifndef KINIT_ALLOC_H
#define KINIT_ALLOC_H

#include <stddef.h>

/* kinit_alloc init */
void kinit_alloc_init(uintptr_t base, size_t limit);

/* kinit_alloc finalize allocations */
void kinit_alloc_finalize(void);

uintptr_t kinit_alloc_get_base(void);
uintptr_t kinit_alloc_get_next(void);
size_t kinit_alloc_get_limit(void);

/* kinit_alloc (x = alignment) */
void* kinit_alloc_align(size_t size, size_t align);

/* kinit_alloc (16 = alignment) */
void* kinit_alloc(size_t size);

#endif
