/* kmmap.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>

#include <kmmap.h>
#include <assert.h>

void kmmap_init(kmmap_t* kmmap) {
	assert(kmmap != NULL);
	memset(kmmap, 0, sizeof(kmmap_t));
}
void kmmap_add(kmmap_t* kmmap, uint64_t address, uint64_t size, uint32_t flags) {
	assert(kmmap != NULL);
	assert(kmmap->count < KMMAP_MAX_REGIONS);
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
