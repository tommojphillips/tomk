/* kernel.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <i86.h>
#include <ps2.h>
#include <multiboot.h>

extern MULTIBOOT_INFO* multiboot_info_ptr; /* entry.asm */

uint32_t aval_memory_total;

void kernel_main(void) {
	uint32_t total_mem_lower = 0;
	uint32_t total_mem_upper = 0;

	if (multiboot_info_ptr->flags & MULTIBOOT_FLAGS_MEM) {
		total_mem_lower = multiboot_info_ptr->mem_lower * 1024;
		total_mem_upper = multiboot_info_ptr->mem_upper * 1024 + 0x100000;
	}
	
	aval_memory_total = total_mem_lower + total_mem_upper;

	printf("TOMK v0.1\n");
	printf("MEMORY: %u Mb\n", aval_memory_total/1024/1024);

	while (1) {
		char ch = getchar();
		if (ch != EOF) {
			printf("%c", ch);
		}
	}
	printf("DONE\n");
}

void kernel_panic(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	hang();
}
