/* kernel.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <kdprint.h>
#include <kernel.h>
#include <linkvars.h>

extern void tty_init(void); /* driver/tty.c */
extern void kshell(void);   /* shell.c */
extern void kmm_init(void); /* kmm.c */

int kver_major = 0;
int kver_minor = 1;

void kernel_main(void) {	
	tty_init();
	kprint("TOMK v%d.%d\n\n", kver_major, kver_minor);

#if 1
	kdprint(".boot   %08X-%08X\n", (uintptr_t)&sec_boot_start, (uintptr_t)&sec_boot_end);
	kdprint(".text   %08X-%08X\n", (uintptr_t)&sec_text_start, (uintptr_t)&sec_text_end);
	kdprint(".rodata %08X-%08X\n", (uintptr_t)&sec_rodata_start, (uintptr_t)&sec_rodata_end);
	kdprint(".data   %08X-%08X\n", (uintptr_t)&sec_data_start, (uintptr_t)&sec_data_end);
	kdprint(".bss    %08X-%08X\n\n", (uintptr_t)&sec_bss_start, (uintptr_t)&sec_bss_end);
#endif

	/* Init Memory Manager */
	kmm_init();
	
	/* Launch kshell (Kernel Test Shell) */
	kshell();

	/* We shouldnt get here. Hang the system */
	kernel_hang();
}

void kernel_panic(const char* fmt, ...) {
	const char* panic_str = "\nKERNEL PANIC\n";
	va_list args;
    va_start(args, fmt);
	fprintf(STDIO, panic_str);
	vfprintf(STDIO, fmt, args);
	fprintf(SERIAL, panic_str);
	vfprintf(SERIAL, fmt, args);
    va_end(args);

	kernel_hang();
}
