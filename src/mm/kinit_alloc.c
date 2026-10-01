/* kinit_alloc.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * Bootstrap bump allocator.
 *
 * Allocations persist for the lifetime of the kernel and cannot be freed.
 * Intended only for early initialization.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <assert.h>
#include <paging.h>
#include <kspacedef.h>

typedef struct kinit_alloc_t {
	uintptr_t base;
	uintptr_t next;
	size_t limit;
	bool enabled;
} kinit_alloc_t;

static kinit_alloc_t ka;

#define DEFAULT_ALIGNMENT 0x10

/* ALIGN */
#define ALIGN(t,x,a) (((t)(x) + (t)((a) - 1)) & ~((t)((a) - 1)))

void kinit_alloc_init(uintptr_t base, uintptr_t limit) {
	assert((base & (PAGE_SIZE-1)) == 0);
	assert((limit & (PAGE_SIZE-1)) == 0);

	ka.base = base;
	ka.next = base;
	ka.limit = limit;
	ka.enabled = true;
}
void kinit_alloc_finalize(void) {
	assert(ka.enabled == true);
	assert(ka.next > ka.base);

	ka.enabled = false;
	ka.limit = ka.next - ka.base;
}
uintptr_t kinit_alloc_get_base(void) {
	return ka.base;
}
uintptr_t kinit_alloc_get_next(void) {
	return ka.next;
}
size_t kinit_alloc_get_limit(void) {
	return ka.limit;
}
void* kinit_alloc_align(size_t size, size_t align) {
	assert(ka.enabled == true);

	size_t a = align; 

	if (a == 0) {
		a = DEFAULT_ALIGNMENT;
	}

	uintptr_t p = ALIGN(uintptr_t, ka.next, a);

	if (((p - ka.base) + size) > ka.limit) {
		return NULL;
	}

	ka.next = p + size;
	
	/* Identity Map virtual address */
	pg_map((p & 0xFFFFF000), V2P(p & 0xFFFFF000), PTE_RW, TO_PAGE((p & 0xFFF) + size + (PAGE_SIZE-1)));
	
	return (void*)p;
}
void* kinit_alloc(size_t size) {
	return kinit_alloc_align(size, 0);
}
