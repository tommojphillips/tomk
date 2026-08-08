
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include <kalloc.h>
#include <kmmap.h>

extern void kernel_hang(void);     /* entry.asm */

void kmmap_init(KMMAP** kmmap, size_t size) {
	*kmmap = kalloc(size);
	(*kmmap)->count = 0;
	(*kmmap)->capacity = (size - sizeof(KMMAP)) / sizeof(KMREGION);
	(*kmmap)->regions = (KMREGION*)((char*)*kmmap + sizeof(KMMAP));
}
void kmmap_add(KMMAP* kmmap, uint32_t address, uint32_t size, uint32_t flags) {
	if (kmmap->capacity < kmmap->count) {
		printf("ERROR: kmmap at capacity\n");
		kernel_hang();
	}
	kmmap->regions[kmmap->count].address = address;
	kmmap->regions[kmmap->count].size = size;
	kmmap->regions[kmmap->count].flags = flags | KMREGION_FLAG_VALID;
	kmmap->count++;
}
void kmmap_remove(KMMAP* kmmap, uint32_t address) {
	for (size_t i = 0; i < kmmap->count; ++i) {
		if (kmmap->regions[i].address == address) {			
			kmmap->regions[i].address = 0;
			kmmap->regions[i].size = 0;
			kmmap->regions[i].flags = 0;
            break;
		}
	}
}
