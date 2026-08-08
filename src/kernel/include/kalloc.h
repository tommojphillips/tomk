/* kernel/include/kalloc.h */

#ifndef KALLOC_H
#define KALLOC_H

extern uint32_t kmem;

void kalloc_init(uint32_t address);
void* kalloc(size_t size);
void kfree(void* ptr);

#endif