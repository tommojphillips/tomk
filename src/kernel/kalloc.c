/* kalloc.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 *
 * Bootstrap bump allocator.
 *
 * Allocations persist for the lifetime of the kernel and cannot be freed.
 * Intended only for early initialization and permanent kernel structures.
 */

#include <stdint.h>
#include <stddef.h>

#include <assert.h>
#include <paging.h>

typedef struct kalloc_t {
	uint32_t base;
	uint32_t next;
	uint32_t limit;
	uint32_t enabled;
} kalloc_t;

static kalloc_t ka;

#define DEFAULT_ALIGNMENT 0x10

/* ALIGN */
#define ALIGN(t,x,a) (((t)(x) + (t)((a) - 1)) & ~((t)((a) - 1)))

void kalloc_init(uint32_t base, uint32_t limit) {
	assert((base & 0x00000FFF) == 0);
	assert((limit & 0x00000FFF) == 0);

	ka.base = base;
	ka.next = base;
	ka.limit = limit;
	ka.enabled = 1;
}
void kalloc_disable(void) {
	ka.enabled = 0;
	ka.limit = ka.next - ka.base;
}
uint32_t kalloc_get_base(void) {
	return ka.base;
}
uint32_t kalloc_get_next(void) {
	return ka.next;
}
uint32_t kalloc_get_limit(void) {
	return ka.limit;
}
void* kalloc_align(size_t size, size_t align) {
	uint32_t p = 0;
	size_t s = 0;

	if (!ka.enabled) {
		return NULL;
	}

	if (align == 0) {
		align = DEFAULT_ALIGNMENT;
	}

	p = ALIGN(uint32_t, ka.next, align);
	s = size;

	if (((p - ka.base) + s) > ka.limit) {
		return NULL;
	}
	
	ka.next = p + s;

	/* Map virtual address */
	pg_map(p + KVIRT, p, PTE_RW, (s + 0xFFF) >> 12);

	return (void*)(p + KVIRT);
}
void* kalloc_page(size_t size) {
	return kalloc_align(size, 0x1000);
}
void* kalloc(size_t size) {
	return kalloc_align(size, 0);
}
