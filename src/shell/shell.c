/* shell.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pmm.h>
#include <vmm.h>
#include <kalloc.h>
#include <kheap.h>
#include <kmalloc.h>
#include <ps2.h>
#include <tty.h>
#include <uart.h>
#include <paging.h>
#include <align.h>
#include <i86.h>
#include <scheduler.h>

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
static int kshell_command_alloctestk(void* userparam);
static int kshell_command_alloctestu(void* userparam);
static int kshell_command_help(void* userparam);
static int kshell_command_pagetables(void* userparam);
static int kshell_command_proc_list(void* userparam);
static int kshell_command_proc_kill(void* userparam);
static int kshell_command_proc_fork(void* userparam);

static void print_memory_stats(int c);
static void print_input(void);
static void alloctestk(void);
static void alloctestu(void);
static void print_paging(void);

static kshell_command_t kshell_commands[] = {
	{ "help", kshell_command_help },
	{ "cls", kshell_command_cls },
	{ "echo", kshell_command_echo },
	{ "mem", kshell_command_mem },
	{ "ver", kshell_command_ver },
	{ "inp", kshell_command_input_print },
	{ "reboot", kshell_command_reboot },
	{ "alloctestk", kshell_command_alloctestk },
	{ "alloctestu", kshell_command_alloctestu },
	{ "pagetables", kshell_command_pagetables },
	{ "procls", kshell_command_proc_list },
	{ "prockill", kshell_command_proc_kill },
	{ "procfork", kshell_command_proc_fork },
};

void kshell(void) {
	char buffer[32+1] = { 0 };
	size_t i = 0;
	size_t x = 0;
	size_t y = 0;

	kprint("\n>");
	tty_get_position(&x, &y);

	while (1) {
		char sc = EOF;
		char ch = EOF;

		haltwait();

		sc = (char)ps2_getscancode();
		if (sc != EOF) {
			ch = (char)ps2_sc2ch(sc);
		}
		else {
			ch = serial_read(COM1);
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
					serial_write(COM1, 0x08);
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
	print_memory_stats(0);
	return 0; /* Success */
}
static int kshell_command_ver(void* userparam) {
	(void)userparam;
	kprint("TOMK v0.1\nKSHELL v0.1\n");
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
static int kshell_command_alloctestk(void* userparam) {
	(void)userparam;
	kdprint("ALLOC TEST\n");
	alloctestk();
	return 0; /* Success */
}
static int kshell_command_alloctestu(void* userparam) {
	(void)userparam;
	kdprint("ALLOC TEST\n");
	alloctestu();
	return 0; /* Success */
}
static int kshell_command_help(void* userparam) {
	(void)userparam;
	kprint("Commands:\n");
	for (size_t i = 0; i < sizeof(kshell_commands) / sizeof(kshell_commands[0]); i++) {
		kprint(" - %s\n", kshell_commands[i].name);
	}

	return 0; /* Success */
}
static int kshell_command_pagetables(void* userparam) {
	(void)userparam;
	print_paging();
	return 0; /* Success */
}
static int kshell_command_proc_list(void* userparam) {
	(void)userparam;
	process_t* head = scheduler_head();
	for (process_t* proc = head; proc != NULL; proc = proc->next) {
		kprintf("proc%u\n", proc->id);
	}
	return 0; /* Success */
}
static int kshell_command_proc_kill(void* userparam) {
	unsigned int id = strtoul(userparam, NULL, 0);
	process_t* head = scheduler_head();
	for (process_t* proc = head; proc != NULL; proc = proc->next) {
		if (proc->id == id) {
			scheduler_unload_kprocess(proc, 1);
			return 0; /* Success */
		}
	}
	return 1; /* Failure */
}
static int kshell_command_proc_fork(void* userparam) {
	unsigned int id = strtoul(userparam, NULL, 0);
	process_t* head = scheduler_head();
	for (process_t* proc = head; proc != NULL; proc = proc->next) {
		if (proc->id == id) {
			scheduler_load_kprocess((process_entry_fn_t)proc->frame.eip);
			return 0; /* Success */
		}
	}
	return 1; /* Failure */
}

static void print_memory_stats(int c) {
	size_t pm_usable = pmm_get_usable();
	size_t pm_used = pmm_get_used();
	size_t pm_free = pmm_get_free();
	
	kprint("\n       |    Physical Memory |\n");
	switch (c) {
		case 0: /* pages */		
		kprint("usable | %12u pages |\n", pm_usable);
		kprint("used   | %12u pages |\n", pm_used);
		kprint("free   | %12u pages |\n", pm_free);
		break;
		
		case 1: /* kb */
		kprint("usable | %15u kb |\n", TO_ADDR(pm_usable) / 1024);
		kprint("used   | %15u kb |\n", TO_ADDR(pm_used) / 1024);
		kprint("free   | %15u kb |\n", TO_ADDR(pm_free) / 1024);
		break;
	}
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
			ch = serial_read(COM1);
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
static void alloctestk(void) {
	print_memory_stats(0);
	kprint("press any key to allocate all kernel memory\n");
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

	print_memory_stats(0);
	kprint("press any key to free all kernel memory\n");
	while (getchar() == EOF);
	
	kprint("freeing....\n");
	for (size_t i = 0; i < (sizeof(ptrs) / sizeof(ptrs[0])); i++) {
		kfree(ptrs[i]);
	}

	print_memory_stats(0);
}
static void alloctestu(void) {
	print_memory_stats(0);
	kprint("press any key to allocate all user memory\n");
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

	print_memory_stats(0);
	kprint("press any key to free all user memory\n");
	while (getchar() == EOF);
	
	kprint("freeing....\n");
	for (size_t i = 0; i < (sizeof(ptrs) / sizeof(ptrs[0])); i++) {
		kfree(ptrs[i]);
	}

	print_memory_stats(0);
}

#define KB ((uintptr_t)1024)
#define MB (KB * 1024)
#define GB (MB * 1024)

#define PD_VIRT() ((uint32_t*)0xFFFFF000)
#define PT_VIRT(pd_idx) ((uint32_t*)(0xFFC00000 + ((pd_idx) << 12)))

static void print_paging(void) {
	/* PD are valid through the recursive mapping */
	uint32_t* pdv = (uint32_t*)0xFFFFF000;

	for (int i = 0; i < 1023; i++) {
		if (pdv[i] == 0) {
			continue;
		}

		/* PT are valid through the recursive mapping */
		uint32_t* ptv = (uint32_t*)(0xFFC00000 + ((uintptr_t)i << 12));

		int start = 0;
		int mapped = (ptv[0] != 0);

		for (int j = 1; j <= 1024; j++) {
			int next_mapped;

			if (j < 1024) {
				next_mapped = (ptv[j] != 0);
			}
			else {
				next_mapped = !mapped;
			}

			if (next_mapped == mapped) {
				continue;
			}

			if (!mapped) {
				start = j;
				mapped = next_mapped;
				continue;
			}

			uintptr_t virt_start = ((uintptr_t)i << 22) + ((uintptr_t)start << 12);
			uintptr_t virt_end = ((uintptr_t)i << 22) + ((uintptr_t)j << 12) - 1;
			uintptr_t phys_start = (uintptr_t)(ptv[start] & 0xFFFFF000);
			uintptr_t phys_end = (uintptr_t)((ptv[j-1] & 0xFFFFF000) + (PAGE_SIZE-1));
			uint32_t flags = ptv[start] & 0xFFF;
			size_t size = virt_end - virt_start + 1;

			kprint("%08X-%08X -> %08X-%08X: %-10s %4u KB\n", virt_start, virt_end, phys_start, phys_end, (flags & PTE_RW) ? "RW" : "RO", size / KB);

			start = j;
			mapped = next_mapped;
		}
	}
}
