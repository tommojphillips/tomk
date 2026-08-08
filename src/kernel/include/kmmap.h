/* kernel/include/kmmap.h */

#ifndef KMMAP_H
#define KMMAP_H

#include <stdint.h>
#include <stddef.h>

#define KMREGION_FLAG_VALID 0x1

#define KMREGION_FLAG_AR_MASK 0x6
#define KMREGION_FLAG_AR_RO   0x0
#define KMREGION_FLAG_AR_RW   0x2
#define KMREGION_FLAG_AR_REV  0x4

#define KMREGION_FLAG_US_MASK  0x8
#define KMREGION_FLAG_US_SUPER 0x0
#define KMREGION_FLAG_US_USER  0x8

typedef struct kmregion_t {
	uint32_t address;
	uint32_t size;
	uint32_t flags;
} kmregion_t;

typedef struct kmmap_t {
	size_t count;
	size_t capacity;
	kmregion_t* regions;
} kmmap_t;

void kmmap_init(kmmap_t** kmmap, size_t size);
void kmmap_add(kmmap_t* kmmap, uint32_t address, uint32_t size, uint32_t flags);
void kmmap_remove(kmmap_t* kmmap, uint32_t address);

#endif
