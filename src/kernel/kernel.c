/* kernel.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <kernel.h>
#include <mb.h>
#include <kalloc.h>
#include <kmmap.h>
#include <pmm.h>
#include <vmm.h>
#include <paging.h>
#include <kheap.h>
#include <i86.h>
#include <ps2.h>
#include <tty.h>
#include <align.h>

/* Linker variables - address */
extern const void* sec_kstart;     /* linker.ld */
extern const void* sec_kend;       /* linker.ld */

extern void pg_init(void);         /* paging.asm */
extern void tty_init(void);        /* driver/tty.c */
extern void pic_init(void);        /* driver/pic.asm */
extern void pit_init(void);        /* driver/pit.c */
extern void ps2_init(void);        /* driver/ps2.asm */

static void print_memory_stats(void);
static void shell(void);

int version_major = 0;
int version_minor = 1;

void kernel_main(void) {
	kmmap_t* kmmap = NULL;
	uint32_t kbase = 0;
	uint32_t kend = 0;
	uint32_t kalloc_base = 0;
	uint32_t ksize = 0;

	tty_init();
	kprint("TOMK v%d.%d\n", version_major, version_minor);

	kbase = (uint32_t)&sec_kstart;
	kalloc_base = (uint32_t)&sec_kend;

	/* Initialize bootstrap allocation system.
	 Give it 5Mb of address space starting from the end of the kernel */
	kalloc_init(kalloc_base, 0x500000);

	/* Init mmap */
	kmmap_init(&kmmap, PAGE_SIZE);
	
	/* Populate kmmap */
	mb_init(kmmap);
	
	/* Setup PD / PMM / VMM */
	pg_init();
	pmm_init(kmmap);
	vmm_init(0x00000000, 0x80000000);

	/* Prevent any components using it from now on */
	kalloc_disable();

	/* Calculate kernel size + kalloc allocations */
	kend = kalloc_get_next();
	ksize = ALIGN(uint32_t, (kend - kbase), PAGE_SIZE);
	
	kprint("[KMAIN] kbase %08X kend %08X\n", kbase, kend);
	
    pmm_mark_used(0x00000000, 0x0009F000); /* LOW MEM (636K) */
    pmm_mark_used(0x000A0000, 0x00020000); /* VGA buffer (128K) */
	pmm_mark_used(kbase, ksize);           /* kernel */
	
    vmm_mark_used(0x00001000, 0x0009F000); /* LOW MEM (636K) */
    vmm_mark_used(0x000A0000, 0x00020000); /* VGA buffer (128K) */
	vmm_mark_used(kbase, ksize);           /* kernel */
	
	pg_map(0x00001000, 0x00001000, (PTE_P | PTE_RW), 0x0009F000 >> 12);
	pg_map(0x000A0000, 0x000A0000, (PTE_P | PTE_RW), 0x00020000 >> 12);
	pg_map(kbase, kbase, (PTE_P | PTE_RW), ksize >> 12);
	
	/* Enable paging */
	pg_enable();
	
	/* Setup kheap */
	kheap_init();

	/* Initialize PIC */
	pic_init();

	/* Initialize PIT */
	pit_init();

	/* Initialize PS/2 keyboard */
	ps2_init();
	
	/* Enable system interrupts */
	enable_interrupts();

#if 0
	// fck round
	void* tst1;
	do {
		tst1 = kmalloc(0x100000);
	} while (tst1);
	kprint(tst1 ? "succ\n" : "fail\n");
	// fck round
#endif

#if 1
	print_memory_stats();
#endif

#if 0
	size_t x, y;
	tty_get_position(&x, &y);
	while (1) {
		char sc = (char)ps2_getscancode();
		char ch = EOF;
		
		if (sc != EOF) {
			tty_set_position(x, y);
			printf("%02X", sc);

			tty_set_position(x+3, y);
			ch = (char)ps2_sc2ch(sc);
			if (ch != EOF) {
				printf("(%c)", ch);
			}
			else {
				printf("(?)", ch);
			}
		}
	}
#endif

#if 1
	shell();
#endif

	kernel_hang();
}

void kprintf(const char* fmt, ...) {
	va_list args;
    va_start(args, fmt);
    vfprintf(STDIO, fmt, args);
	vfprintf(SERIAL, fmt, args);
    va_end(args);
}
void kdprintf(const char* fmt, ...) {
	va_list args;
    va_start(args, fmt);
	vfprintf(SERIAL, fmt, args);
    va_end(args);
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

static int kshell_command_cls(void* userparam);
static int kshell_command_echo(void* userparam);
static int kshell_command_mem(void* userparam);
static int kshell_command_ver(void* userparam);

typedef struct kshell_command_t {
	const char* name;
    int (*cmd)(void*);
} kshell_command_t;

static kshell_command_t kshell_commands[] = {
	{ "CLS", kshell_command_cls },
	{ "ECHO", kshell_command_echo },
	{ "MEM", kshell_command_mem },
	{ "VER", kshell_command_ver },
};

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
	print_memory_stats();
	return 0; /* Success */
}
static int kshell_command_ver(void* userparam) {
	(void)userparam;
	printf("TOMK v%d.%d\n", version_major, version_minor);
	return 0; /* Success */
}

static void shell(void) {
	char buffer[32] = { 0 };
	size_t i = 0;
	size_t x, y;

	tty_putc('\n');
	tty_putc('>');
	tty_get_position(&x, &y);

	while (1) {
		char sc = (char)ps2_getscancode();
		char ch = EOF;
		
		if (sc != EOF) {
			ch = (char)ps2_sc2ch(sc);
			if (ch != EOF) {
				if (i >= 32) {
					continue;
				}
				buffer[i++] = ch;
				tty_putc(ch);

				if (i < 32) {
					buffer[i] = '\0';
				}
			}
			else {

				switch (sc) {
					case 0x00:
						break;
					case 0x01: /* Escape */
						tty_putc('\n');
						tty_putc('>');
						tty_get_position(&x, &y);
						i = 0;
						buffer[0] = '\0';
						break;
					case 0x0E: /* Backspace */						
						if (i < 1) {
							continue;
						}
						i--;
						buffer[i] = '\0';
						tty_set_position(x-1, y);
						tty_putc(' ');
						tty_set_position(x-1, y);
						break;
					case 0x0F: /* Tab */
						break;
					case 0x1C: /* Enter */
						if (buffer[0]) {
							int found = 0;
							char cmd[32] = { 0 };
							char args[32] = { 0 };
							size_t i;							
							for (i = 0; i < strlen(buffer); i++) {
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
								if (strcmp(kshell_commands[i].name, cmd) == 0) {
									tty_putc('\n');
									kshell_commands[i].cmd(args);
									found = 1;
									break;
								}
							}
							if (!found) {
								tty_putc('\n');
								tty_putc('e');
								tty_putc('r');
								tty_putc('r');
								tty_putc(':');
								tty_putc(' ');
								tty_putc('u');
								tty_putc('n');
								tty_putc('k');
								tty_putc(' ');
								tty_putc('c');
								tty_putc('m');
								tty_putc('d');
							}
						}						
						tty_putc('\n');
						tty_putc('>');
						tty_get_position(&x, &y);
						i = 0;
						buffer[0] = '\0';
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
}
static void print_memory_stats(void) {	
	uint32_t ka_base = kalloc_get_base();
	uint32_t ka_next = kalloc_get_next();
	uint32_t ka_limit = ALIGN(uint32_t, kalloc_get_limit(), PAGE_SIZE);
	uint32_t ka_size = ALIGN(uint32_t, (ka_next - ka_base), PAGE_SIZE);
	
	uint32_t pm_usable = pmm_get_usable();
	uint32_t pm_used = pmm_get_used();
	uint32_t pm_free = pmm_get_free();
	
	uint32_t vm_usable = vmm_get_usable();
	uint32_t vm_used = vmm_get_used();
	uint32_t vm_free = vmm_get_free();
	
	uint32_t ka_usable = ((ka_base + ka_limit) - ka_base) >> 12;
	uint32_t ka_used = ka_size >> 12;
	uint32_t ka_free = ka_usable - ka_used;
	
	printf("\nMARK     |   KALLOC |      PMM |      VMM \n");
	printf("usable   | %8u | %8u | %8u\n", ka_usable, pm_usable, vm_usable);
	printf("used     | %8u | %8u | %8u\n", ka_used, pm_used, vm_used);
	printf("free     | %8u | %8u | %8u\n", ka_free, pm_free, vm_free);
}
