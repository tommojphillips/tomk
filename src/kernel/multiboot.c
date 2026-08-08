/* multiboot.c */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include <multiboot.h>
#include <kmmap.h>
#include <kernel.h>

extern MULTIBOOT_INFO* multiboot_info_ptr; /* entry.asm */

void mb_init(KMMAP* kmmap, uint32_t* out_memory_total) {
	if (multiboot_info_ptr->flags & MULTIBOOT_FLAGS_MMAP) {
		MULTIBOOT_MMAP* mmap = (MULTIBOOT_MMAP*)multiboot_info_ptr->mmap_addr;
		uint32_t memory_total = 0;
		
		while ((uintptr_t)mmap < multiboot_info_ptr->mmap_addr + multiboot_info_ptr->mmap_length) {
			uint64_t addr = ((uint64_t)mmap->addr2 << 32) |  (uint64_t)mmap->addr1;
			uint64_t len = ((uint64_t)mmap->len2 << 32) |  (uint64_t)mmap->len1;
						
			if (mmap->type == MULTIBOOT_MMAP_TYPE_RAM) {
				memory_total += len;
				kmmap_add(kmmap, (uint32_t)addr, (uint32_t)len, KMREGION_FLAG_AR_RW);
			}
			else {
				kmmap_add(kmmap, (uint32_t)addr, (uint32_t)len, KMREGION_FLAG_AR_REV);
			}
			
			mmap = (MULTIBOOT_MMAP*)((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
		}
		
		if (out_memory_total) {
			*out_memory_total = memory_total;
		}
	}
    else {
        printf("ERROR: Multiboot mmap not present\n");
        kernel_hang();
    }
}
