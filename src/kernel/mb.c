/* mb.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include <mb.h>
#include <kmmap.h>
#include <kernel.h>

extern multiboot_info_t* multiboot_info_ptr; /* entry.asm */

void mb_init(kmmap_t* kmmap) {
	if (multiboot_info_ptr->flags & MULTIBOOT_FLAGS_MMAP) {
		multiboot_mmap_t* mmap = (multiboot_mmap_t*)multiboot_info_ptr->mmap_addr;
		
		while ((uintptr_t)mmap < multiboot_info_ptr->mmap_addr + multiboot_info_ptr->mmap_length) {
			uint64_t addr = ((uint64_t)mmap->addr2 << 32) |  (uint64_t)mmap->addr1;
			uint64_t len = ((uint64_t)mmap->len2 << 32) |  (uint64_t)mmap->len1;
			
			if (mmap->type == MULTIBOOT_MMAP_TYPE_RAM) {
				kmmap_add(kmmap, addr, len, KMREGION_TYPE_RAM);
			}
			else {
				kmmap_add(kmmap, addr, len, KMREGION_TYPE_REV);
			}
			
			mmap = (multiboot_mmap_t*)((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
		}
	}
    else {
        kernel_panic("ERROR: Multiboot mmap not present\n");
    }
}
