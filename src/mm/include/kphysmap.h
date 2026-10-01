/* kphysmap.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KPHYSMAP_H
#define KPHYSMAP_H

#include <stdint.h>
#include <stddef.h>

#define KPHYSREGION_VALID     0x1

#define KPHYSREGION_TYPE_MASK 0x6
#define KPHYSREGION_TYPE_RAM  0x2 /* RAM */
#define KPHYSREGION_TYPE_REV  0x4 /* Reserved */

#define KPHYSMAP_MAX_REGIONS  10

/* Kernel Memory Region */
typedef struct _kphysregion {
	uintptr_t address;
	size_t size;
	uint32_t flags;
} kphysregion_t;

/* Kernel Memory Map */
typedef struct _kphysmap {
	size_t count;
	kphysregion_t regions[KPHYSMAP_MAX_REGIONS];
} kphysmap_t;

void kphysmap_init(kphysmap_t* kmmap);
void kphysmap_add(kphysmap_t* kmmap, uintptr_t address, size_t size, uint32_t flags);
void kphysmap_remove(kphysmap_t* kmmap, uintptr_t address);

#endif
