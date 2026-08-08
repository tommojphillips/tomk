
#include <stdint.h>
#include <stddef.h>

uint32_t kmem;

void kalloc_init(uint32_t address) {
	kmem = address;
}
void* kalloc(size_t size) {
	void* ptr = (void*)kmem;
	kmem += size;
	return ptr;
}
void kfree(void* ptr) {
	/* no need to free kernel allocations, yet */
	(void)ptr;
}
