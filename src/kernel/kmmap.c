/* kmmap.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>

#include <kmmap.h>
#include <kernel.h>
#include <assert.h>
#include <kalloc.h>

void kmmap_init(kmmap_t** kmmap, size_t size) {
	assert(kmmap != NULL);
	*kmmap = kalloc(size);	
	assert(*kmmap != NULL);
	(*kmmap)->count = 0;
	(*kmmap)->capacity = (size - sizeof(kmmap_t)) / sizeof(kmregion_t);
	(*kmmap)->regions = (kmregion_t*)((char*)*kmmap + sizeof(kmmap_t));
}
void kmmap_add(kmmap_t* kmmap, uint64_t address, uint64_t size, uint32_t flags) {
	assert(kmmap != NULL);
	assert(kmmap->count < kmmap->capacity);
	kmmap->regions[kmmap->count].address = address;
	kmmap->regions[kmmap->count].size = size;
	kmmap->regions[kmmap->count].flags = flags | KMREGION_VALID;
	kmmap->count++;
}
void kmmap_remove(kmmap_t* kmmap, uint64_t address) {
	assert(kmmap != NULL);
	for (size_t i = 0; i < kmmap->count; ++i) {
		if (kmmap->regions[i].address == address) {			
			kmmap->regions[i].address = 0;
			kmmap->regions[i].size = 0;
			kmmap->regions[i].flags = 0;
            break;
		}
	}
}
