/* kalloc.c
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

	uintptr_t p = 0;
	size_t s = 0;

	if (align == 0) {
		align = DEFAULT_ALIGNMENT;
	}

	p = ALIGN(uintptr_t, ka.next, align);
	s = size;

	if (((p - ka.base) + s) > ka.limit) {
		return NULL;
	}
	
	ka.next = p + s;

	/* Map virtual address */
	pg_map(p + KVIRT, p, PTE_RW, PAGE_COUNT(s + (PAGE_SIZE-1)));

	return (void*)(p + KVIRT);
}
void* kinit_alloc(size_t size) {
	return kinit_alloc_align(size, 0);
}
