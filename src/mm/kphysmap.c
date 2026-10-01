/* kphysmap.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <kphysmap.h>
#include <assert.h>

void kphysmap_init(kphysmap_t* map) {
	assert(map != NULL);
	memset(map, 0, sizeof(kphysmap_t));
}
void kphysmap_add(kphysmap_t* map, uintptr_t address, size_t size, uint32_t flags) {
	assert(map != NULL);
	assert(map->count < KPHYSMAP_MAX_REGIONS);
	map->regions[map->count].address = address;
	map->regions[map->count].size = size;
	map->regions[map->count].flags = flags | KPHYSREGION_VALID;
	map->count++;
}
void kphysmap_remove(kphysmap_t* map, uintptr_t address) {
	assert(map != NULL);
	for (size_t i = 0; i < map->count; ++i) {
		if (map->regions[i].address == address) {			
			map->regions[i].address = 0;
			map->regions[i].size = 0;
			map->regions[i].flags = 0;
            break;
		}
	}
}
