/* kmmap.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KMMAP_H
#define KMMAP_H

#include <stdint.h>
#include <stddef.h>

#define KMREGION_VALID     0x1

#define KMREGION_TYPE_MASK 0x6
#define KMREGION_TYPE_RAM  0x2 /* RAM */
#define KMREGION_TYPE_REV  0x4 /* Reserved */

#define KMMAP_MAX_REGIONS  10

/* Kernel Memory Region */
typedef struct kmregion_t {
	uint64_t address;
	uint64_t size;
	uint32_t flags;
} kmregion_t;

/* Kernel Memory Map */
typedef struct kmmap_t {
	size_t count;
	kmregion_t regions[KMMAP_MAX_REGIONS];
} kmmap_t;

void kmmap_init(kmmap_t* kmmap);
void kmmap_add(kmmap_t* kmmap, uint64_t address, uint64_t size, uint32_t flags);
void kmmap_remove(kmmap_t* kmmap, uint64_t address);

#endif
