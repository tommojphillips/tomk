/* kalloc.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>

#include <kernel.h>

#define KDBG
#ifdef KDBG
#include <stdio.h>
#define kprint(...) printf(__VA_ARGS__)
#else
#define kprint(...)
#endif

#define KPANIC

static uint32_t kalloc_base;
static uint32_t kalloc_next;
static uint32_t kalloc_limit;

#define DEFAULT_ALIGNMENT 0x10

void kalloc_init(uint32_t base, uint32_t limit) {
	kalloc_base = base;
	kalloc_next = base;
	kalloc_limit = limit;
}
uint32_t kalloc_get_base(void) {
	return kalloc_base;
}
uint32_t kalloc_get_next(void) {
	return kalloc_next;
}
uint32_t kalloc_get_limit(void) {
	return kalloc_limit;
}
void* kalloc_align(size_t size, size_t align) {
	if (align > 0x1000) {
		align = 0x1000;
	}
	if (align == 0) {
		align = DEFAULT_ALIGNMENT;
	}
	size = (size + (align - 1)) & ~((size_t)(align - 1));
	if (kalloc_next + size > kalloc_base + kalloc_limit) {
	#ifdef KPANIC
		kernel_panic("[KALLOC] Error: Out of memory!\n");
	#else
		kprint("[KALLOC] Error: Out of memory!\n");
	#endif
		return NULL;
	}
	void* ptr = (void*)kalloc_next;
	kalloc_next += size;
	kprint("[KALLOC] Alloc p=0x%08X (0x%X)\n", (uint32_t)ptr, size);
	return ptr;
}
void* kalloc_page(size_t size) {
	return kalloc_align(size, 0x1000);
}
void* kalloc(size_t size) {
	return kalloc_align(size, DEFAULT_ALIGNMENT);
}
void kfree(void* ptr) {
	/* no need to free kernel allocations, yet */
	(void)ptr;
	kprint("[KALLOC] Free not managed! p=0x%08X\n", (uint32_t)ptr);
}
