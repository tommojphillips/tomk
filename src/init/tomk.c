/* kernel.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <linkvars.h>
#include <kspacedef.h>
#include <kdprint.h>

#include <kernel.h>
#include <scheduler.h>
#include <mminit.h>
#include <gdt.h>

extern void tty_init(void); /* driver/tty.c */
extern void kshell(void);   /* shell.c */

void kmain(void) {
	tty_init();
	kprint("TOMK v0.1\n\n");

#if 1
	kdprint("Sections:\n");
	kdprint(" .boot   %08X-%08X\n", (uintptr_t)&sec_boot_start, (uintptr_t)&sec_boot_end);
	kdprint(" .text   %08X-%08X\n", V2P((uintptr_t)&sec_text_start), V2P((uintptr_t)&sec_text_end));
	kdprint(" .rodata %08X-%08X\n", V2P((uintptr_t)&sec_rodata_start), V2P((uintptr_t)&sec_rodata_end));
	kdprint(" .data   %08X-%08X\n", V2P((uintptr_t)&sec_data_start), V2P((uintptr_t)&sec_data_end));
	kdprint(" .bss    %08X-%08X\n\n", V2P((uintptr_t)&sec_bss_start), V2P((uintptr_t)&sec_bss_end));
#endif

#if 1
	kdprint("Selectors:\n");
	kdprint(" KCODE  %04X\n", KCODE);
	kdprint(" KDATA  %04X\n", KDATA);
	kdprint(" KSTACK %04X\n", KSTACK);
	kdprint(" UCODE  %04X\n", UCODE);
	kdprint(" UDATA  %04X\n", UDATA);
	kdprint(" USTACK %04X\n\n", USTACK);
#endif

	/* Init memory stack */
	mminit();

	/* Init scheduler */
	scheduler_init();

	/* Spin up kshell process */
	kprint("Starting KSHELL...\n");
	scheduler_load_kprocess((uintptr_t)kshell);
	
	/* Unload ourself */
	scheduler_unload_kprocess(scheduler_current(), 0);
}

void kpanic(const char* fmt, ...) {
	const char* panic_str = "\nKERNEL PANIC\n";
	va_list args;
    va_start(args, fmt);
	fprintf(STDIO, panic_str);
	vfprintf(STDIO, fmt, args);
	fprintf(SERIAL, panic_str);
	vfprintf(SERIAL, fmt, args);
    va_end(args);

	khang();
}
