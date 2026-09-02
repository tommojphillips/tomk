/* kmm.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 * Initialise the kernel memory-management stack:
 *
 *  kinit_alloc -> PMM -> VMM -> KHEAP
 * 
 * The Multiboot memory map is first parsed to determine the available
 * physical memory. A temporary bump allocator is then initialised so
 * that PMM and VMM can allocate the storage required for their internal
 * bitmaps before the kernel heap is available.
 * 
 * PMM is initialised from the Multiboot memory map and uses kinit_alloc
 * for its bitmap. VMM is then initialised over the kernel virtual-memory
 * region and likewise uses kinit_alloc for its bitmap.
 * 
 * Once PMM and VMM are operational, the temporary allocator is finalised.
 * Its allocations, along with the kernel image and VGA buffer, are then
 * marked as used in both the physical and virtual memory managers. This
 * prevents subsequent allocations from overlapping memory that is already
 * in use.
 * 
 * With the initial memory state established, the kernel heap can be
 * initialised on top of the VMM.	
 */
 
 /* Setup kernel memory allocation stack (PMM -> VMM -> KHEAP)
 * First we need to read the memory map passed from multiboot to
 * to determine how much RAM the system has. Then we need to setup
 * a temp allocation system for allocating the bitmaps for the virtual and physical memory
 * managers (vmm/pmm). 
 * PMM is setup by passing the mmap. PMM uses kinit_alloc to allocate memory for it's bitmap.
 * VMM is setup by passing a virtual start and end address for the virtual memory region.
 * Once the temp allocator, PMM and VMM are setup we finalize the temp allocator, disabling it.
 * Now we mark the kernel image, VGA buffer, and kinit_alloc allocations as 'USED' in both, the
 * PMM and the VMM. This prevents the PMM/VMM from doling out memory that is in use. The heap is
 * ready to be setup. 
 */

#include <stdint.h>
#include <stddef.h>

#include <kernel.h>
#include <kmmap.h>
#include <kheap.h>
#include <kalloc.h>
#include <pmm.h>
#include <vmm.h>
#include <mb.h>
#include <paging.h>
#include <linkvars.h>
#include <align.h>

void kmm_init(void) {
	uintptr_t kbase = 0;
	uintptr_t kend = 0;
	uintptr_t ksize = 0;
	size_t kinit_alloc_size = 0;
	uintptr_t kinit_alloc_base = 0;
	uintptr_t kinit_alloc_end = 0;
	kmmap_t kmmap = { 0 };

	kbase = (uintptr_t)&sec_kstart + KVIRT;
	kend = ALIGN(uintptr_t, (uintptr_t)&sec_kend, PAGE_SIZE);

	/* Init mmap */
	kmmap_init(&kmmap);
	
	/* Populate kmmap */
	mb_init(&kmmap);

	/* Init kernel initialization allocator */
	kinit_alloc_init(kend - KVIRT, 0x500000);

	/* Init physical memory allocator */
	pmm_init(&kmmap);

	/* Init virtual memory allocator */
	vmm_init(KVIRT, 0xFFFFF000);

	/* Disable bump allocator */
	kinit_alloc_finalize();

	/* Calculate kernel size + kinit_alloc allocations */
	ksize = ALIGN(uintptr_t, (kend - kbase), PAGE_SIZE);
	kinit_alloc_base = kinit_alloc_get_base();
	kinit_alloc_end = kinit_alloc_get_next();
	kinit_alloc_size = ALIGN(uintptr_t, (kinit_alloc_end - kinit_alloc_base), PAGE_SIZE);
	
	pmm_mark_used(0x000A0000, 0x20000);              /* Mark VGA buffer used */
	pmm_mark_used(kbase - KVIRT, ksize);             /* Mark kernel image used */
	pmm_mark_used(kinit_alloc_base, kinit_alloc_size); /* Mark kinit_alloc allocations used */
    
	vmm_mark_used(0x000A0000 + KVIRT, 0x20000);      /* Mark VGA buffer at KVIRT used */
	vmm_mark_used(kbase, ksize);                     /* Mark kernel image used */
	vmm_mark_used(kinit_alloc_base + KVIRT, kinit_alloc_size); /* Mark kinit_alloc allocations used */

	/* Init kernel heap allocator */
	kheap_init();
}
