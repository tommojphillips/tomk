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
	kdprint(" .text   %08X-%08X\n", (uintptr_t)&sec_text_start, (uintptr_t)&sec_text_end);
	kdprint(" .rodata %08X-%08X\n", (uintptr_t)&sec_rodata_start, (uintptr_t)&sec_rodata_end);
	kdprint(" .data   %08X-%08X\n", (uintptr_t)&sec_data_start, (uintptr_t)&sec_data_end);
	kdprint(" .bss    %08X-%08X\n\n", (uintptr_t)&sec_bss_start, (uintptr_t)&sec_bss_end);
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

	/* Spin up .kshell proc */
	kprint("Starting .kshell...\n");
	scheduler_load_kprocess(".kshell", kshell);

	/* Unload self */
	kprint("Unloading .kinit...\n");
	scheduler_unload_process(scheduler_current());

	/* Wait until .kinit proc is unloaded */
	halt();
}
