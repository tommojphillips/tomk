/* mb.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>

#include <mb.h>
#include <kphysmap.h>
#include <assert.h>
#include <kspacedef.h>

extern multiboot_info_t* mb_info_ptr; /* entry.asm */

void mb_init(kphysmap_t* map) {
	assert(mb_info_ptr != NULL);
	assert(mb_info_ptr->flags & MULTIBOOT_FLAGS_MMAP);

	uintptr_t addr = P2V(mb_info_ptr->mmap_addr);
	multiboot_mmap_t* mmap = (multiboot_mmap_t*)addr;
	while ((uintptr_t)mmap < addr + mb_info_ptr->mmap_length) {
		uint64_t addr = ((uint64_t)mmap->addr2 << 32) | (uint64_t)mmap->addr1;
		uint64_t len = ((uint64_t)mmap->len2 << 32) | (uint64_t)mmap->len1;
		
		if (mmap->type == MULTIBOOT_MMAP_TYPE_RAM) {
			kphysmap_add(map, addr, len, KPHYSREGION_TYPE_RAM);
		}
		else {
			kphysmap_add(map, addr, len, KPHYSREGION_TYPE_REV);
		}
		
		mmap = (multiboot_mmap_t*)((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
	}
}
