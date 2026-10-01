/* mman.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>

/* mmap
 virt:   Start virtual address to map, if NULL uses an arbitrary virtual address to map the physical address(s)
 phys:   Start physical address to map, if NULL uses an arbitrary physical address to map the virtual address(s)
 length: Length in bytes
 flags:  Page permissions 
 Returns: The mapped virtual address */
void* mmap(void* virt_addr, uintptr_t phys_addr, size_t length, uint32_t flags);

/* munmap 
 virt:   Start virtual address to unmap
 length: Length in bytes */
void munmap(void* virt, size_t length);
