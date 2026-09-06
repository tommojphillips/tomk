/* shell.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <pmm.h>
#include <vmm.h>
#include <kalloc.h>
#include <kheap.h>
#include <ps2.h>
#include <tty.h>
#include <uart.h>
#include <paging.h>
#include <align.h>
#include <i86.h>

#include <kdprint.h>
#include <kernel.h>

typedef struct kshell_command_t {
	const char* name;
    int (*cmd)(void*);
} kshell_command_t;

static int kshell_command_cls(void* userparam);
static int kshell_command_echo(void* userparam);
static int kshell_command_mem(void* userparam);
static int kshell_command_ver(void* userparam);
static int kshell_command_input_print(void* userparam);
static int kshell_command_reboot(void* userparam);
static int kshell_command_alloctest(void* userparam);
static int kshell_command_get_cpu_state(void* userparam);

static kshell_command_t kshell_commands[] = {
	{ "CLS", kshell_command_cls },
	{ "ECHO", kshell_command_echo },
	{ "MEM", kshell_command_mem },
	{ "VER", kshell_command_ver },
	{ "INP", kshell_command_input_print },
	{ "REBOOT", kshell_command_reboot },
	{ "ALLOCTEST", kshell_command_alloctest },
	{ "GETSTATE", kshell_command_get_cpu_state },
};

static void print_memory_stats(void);
static void print_input(void);
static void alloctest(void);

void kshell(void) {
	char buffer[32+1] = { 0 };
	size_t i = 0;
	size_t x, y;

	kprint("\n>");
	tty_get_position(&x, &y);

	while (1) {
		char sc = EOF;
		char ch = EOF;

		sc = (char)ps2_getscancode();
		if (sc != EOF) {
			ch = (char)ps2_sc2ch(sc);
		}
		else {
			ch = serial_read();
		}

		if (ch != EOF) {
			switch (ch) {
				case 0x08: /* Backspace */						
					if (i < 1) {
						continue;
					}
					i--;
					buffer[i] = '\0';
					tty_set_position(x+i, y);
					serial_write(0x08);
					break;

				case 0x0D: /* Enter */	
				if (buffer[0]) {
					char cmd[32+1] = { 0 };
					char args[32+1] = { 0 };
					int found = 0;
					size_t i = 0;							
					for (; i < strlen(buffer); i++) {
						if (buffer[i] == ' ') {
							i++;
							break;
						}
						if (buffer[i] == '\0') {
							break;
						}
						cmd[i] = buffer[i];
					}
					strcpy(args, buffer + i);

					for (i = 0; i < sizeof(kshell_commands) / sizeof(kshell_commands[0]); i++) {
						if (stricmp(kshell_commands[i].name, cmd) == 0) {
							kprint("\n");
							kshell_commands[i].cmd(args);
							found = 1;
							break;
						}
					}
					if (!found) {
						kprint("\nerr: unk cmd");
					}
				}
				kprint("\n>");
				tty_get_position(&x, &y);
				i = 0;
				buffer[0] = '\0';
				break;

				default:				
				if (i > 31) {
					continue;
				}
				buffer[i++] = ch;

				/* echo character */
				kprint("%c", ch);

				if (i < 32) {
					buffer[i] = '\0';
				}
				break;
			}
		}
		else {

			switch (sc) {
				case 0x00:
					break;
				case 0x01: /* Escape */
					kprint("\n>");
					tty_get_position(&x, &y);
					i = 0;
					buffer[0] = '\0';
					break;
				case 0x0E: /* Backspace */
					break;
				case 0x0F: /* Tab */
					break;
				case 0x1C: /* Enter */					
					break;
				case 0x1D: /* Ctrl */
					break;
				case 0x2A: /* Left-Shift */
					break;
				case 0x36: /* Right-Shift */
					break;
				case 0x38: /* Alt */
					break;
				case 0x48: /* Up Arrow */
					break;
				case 0x50: /* Down Arrow */
					break;
				case 0x4B: /* Left Arrow */
					i--;
					tty_set_position(x-1, y);
					break;
				case 0x4D: /* Right Arrow */
					i++;
					tty_set_position(x+1, y);
					break;
				case 0x3B: /* F1 */
					break;
				case 0x3C: /* F2 */
					break;
				case 0x3D: /* F3 */
					break;
				case 0x3E: /* F4 */
					break;
				case 0x3F: /* F5 */
					break;
				case 0x40: /* F6 */
					break;
				case 0x41: /* F7 */
					break;
				case 0x42: /* F8 */
					break;
				case 0x43: /* F9 */
					break;
				case 0x44: /* F10 */
					break;
				case 0x57: /* F11 */
					break;
				case 0x58: /* F12 */
					break;			
			}			
		}		
	}
}

static int kshell_command_cls(void* userparam) {
	(void)userparam;
	tty_clear_screen();
	return 0; /* Success */
}
static int kshell_command_echo(void* userparam) {
	const char* s = userparam;
	while(*s) {
		tty_putc(*s);
		s++;
	}
	return 0; /* Success */
}
static int kshell_command_mem(void* userparam) {
	(void)userparam;
	kdprint("MEMORY\n");
	print_memory_stats();
	return 0; /* Success */
}
static int kshell_command_ver(void* userparam) {
	(void)userparam;
	kprint("TOMK v%d.%d\n", kver_major, kver_minor);
	return 0; /* Success */
}
static int kshell_command_input_print(void* userparam) {
	(void)userparam;
	kdprint("INPUT PRINT\n");
	print_input();
	return 0; /* Success */
}
static int kshell_command_reboot(void* userparam) {
	(void)userparam;
	kdprint("REBOOT\n");
	ps2_cpu_reset();
	return 0; /* Success */
}
static int kshell_command_alloctest(void* userparam) {
	(void)userparam;
	kdprint("ALLOC TEST\n");
	alloctest();
	return 0; /* Success */
}
static int kshell_command_get_cpu_state(void* userparam) {
	(void)userparam;
	cpu_state_t state = { 0 };
	getstate(&state);
	kprint("eax=%08X ebx=%08X ecx=%08X edx=%08X\n", state.eax, state.ebx, state.ecx, state.edx);
	kprint("esi=%08X edi=%08X ebp=%08X esp=%08X\n", state.esi, state.edi, state.ebp, state.esp);
	kprint("es =%8.4X cs =%8.4X ss =%8.4X ds =%8.4X\n", state.es, state.cs, state.ss, state.ds);
	kprint("fs =%8.4X gs =%8.4X\n", state.fs, state.gs);
	kprint("cr0=%08X cr2=%08X cr3=%08X\n", state.cr0, state.cr2, state.cr3);
	kprint("eip=%08X eflags=%08X\n", state.eip, state.eflags);

	return 0; /* Success */
}

static void print_memory_stats(void) {	
	uintptr_t ka_base = kinit_alloc_get_base();
	uintptr_t ka_next = kinit_alloc_get_next();
	size_t ka_limit = ALIGN(size_t, kinit_alloc_get_limit(), PAGE_SIZE);
	size_t ka_size = ALIGN(size_t, (ka_next - ka_base), PAGE_SIZE);
	
	size_t pm_usable = pmm_get_usable();
	size_t pm_used = pmm_get_used();
	size_t pm_free = pmm_get_free();
	
	size_t vm_usable = vmm_get_usable();
	size_t vm_used = vmm_get_used();
	size_t vm_free = vmm_get_free();
	
	size_t ka_usable = PAGE_COUNT(ka_limit);
	size_t ka_used = PAGE_COUNT(ka_size);
	size_t ka_free = ka_usable - ka_used;
	
	size_t st_usable = PAGE_COUNT((size_t)&kstack_top - (size_t)&kstack_base);
	size_t st_used = PAGE_COUNT((size_t)&kstack_top - (size_t)getesp() + (PAGE_SIZE-1));
	size_t st_free = st_usable - st_used;

	kprint("\nMARK     |    STACK |   KALLOC |      PMM |      VMM\n");
	kprint("usable   | %8u | %8u | %8u | %8u\n", st_usable, ka_usable, pm_usable, vm_usable);
	kprint("used     | %8u | %8u | %8u | %8u\n", st_used, ka_used, pm_used, vm_used);
	kprint("free     | %8u | %8u | %8u | %8u\n", st_free, ka_free, pm_free, vm_free);
}
static void print_input(void) {
	size_t x, y;
	tty_get_position(&x, &y);
	while (1) {
		char sc = EOF;
		char ch = EOF;

		sc = (char)ps2_getscancode();
		if (sc != EOF) {
			ch = (char)ps2_sc2ch(sc);
		}
		else {
			ch = serial_read();
		}
		
		tty_set_position(x, y);
		if (sc != EOF) {
			kprint("%02X ", sc);

		}

		tty_set_position(x+3, y);
		if (ch != EOF) {
			kprint("(%02X)\n", ch);
		}
	}
}
static void alloctest(void) {
	print_memory_stats();
	kprint("press any key to allocate all memory\n");
	while (getchar() == EOF);

	void* ptrs[8192] = { 0 };
	kprint("allocating....\n");
	for (size_t i = 0; i < (sizeof(ptrs) / sizeof(ptrs[0])); i++) {
		ptrs[i] = kmalloc(0x40000);
		if (ptrs[i]) {
			memset(ptrs[i], 0, 0x40000);
		}
		else {
			break;
		}
	}

	print_memory_stats();
	kprint("press any key to free all memory\n");
	while (getchar() == EOF);
	
	kprint("freeing....\n");
	for (size_t i = 0; i < (sizeof(ptrs) / sizeof(ptrs[0])); i++) {
		kfree(ptrs[i]);
	}

	print_memory_stats();
}
