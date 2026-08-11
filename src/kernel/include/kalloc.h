/* kalloc.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KALLOC_H
#define KALLOC_H

/* kalloc init */
void kalloc_init(uint32_t base, uint32_t limit);

/* Get bottom of HEAP */
uint32_t kalloc_get_base(void);
uint32_t kalloc_get_next(void);
uint32_t kalloc_get_limit(void);

/* kalloc (x = alignment) */
void* kalloc_align(size_t size, size_t align);

/* kalloc (4096 = alignment), */
void* kalloc_page(size_t size);

/* kalloc (16 = alignment) */
void* kalloc(size_t size);
void kfree(void* ptr);

#endif
