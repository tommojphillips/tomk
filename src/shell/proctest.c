/* kernel.c
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <kdprint.h>
#include <scheduler.h>
#include <tty.h>
#include <vga.h>
#include <i86.h>

void proc_race(char ch, uint8_t col, size_t sx, size_t sy) {
	size_t x = sx;
	size_t y = sy;
	int t = 0;
	int j = 0;

	while(1) {
		tty_pute(ch, col, x, y);
		while (t++ < 10) {
			haltwait();
		}
		t = 0;
		tty_pute(' ', col, x, y);
		
		if (j) {
			if (x > 0) {
				x--;
			}
			else {
				j = 0;
			}
		}
		else {
			if (x < VGA_WIDTH - 1) {
				x++;
			}
			else {
				j = 1;
			}
		}
	}
}
void proc_a(void) {
	proc_race('A', 1, 0, 4);
}
void proc_b(void) {
	proc_race('B', 2, 0, 5);
}
void proc_c(void) {
	proc_race('C', 3, 0, 6);
}
void proc_d(void) {
	proc_race('D', 4, 0, 7);
}
void proc_e(void) {
	proc_race('E', 5, 0, 8);
}
void proc_f(void) {
	proc_race('F', 6, 0, 9);
}
void proc_g(void) {
	proc_race('G', 9, 0, 10);
}
void proc_h(void) {
	proc_race('H', 10, 0, 11);
}
void proc_i(void) {
	proc_race('I', 11, 0, 12);
}
void proc_j(void) {
	proc_race('J', 12, 0, 13);
}
void proc_k(void) {
	proc_race('K', 13, 0, 14);
}
void proc_l(void) {
	proc_race('L', 14, 0, 15);
}
void proc_m(void) {
	proc_race('M', 15, 0, 16);
}
void proc_n(void) {
	proc_race('N', 8, 0, 17);
}

void proctest(void) {

	/* Spin up proc_a process */
	kprint("Starting proc_a...\n");
	scheduler_load_kprocess("A", proc_a);

	/* Spin up proc_b process */
	kprint("Starting proc_b...\n");
	scheduler_load_kprocess("B", proc_b);

	/* Spin up proc_c process */
	kprint("Starting proc_c...\n");
	scheduler_load_kprocess("C", proc_c);

	/* Spin up proc_d process */
	kprint("Starting proc_d...\n");
	scheduler_load_kprocess("D", proc_d);

	/* Spin up proc_e process */
	kprint("Starting proc_e..\n");
	scheduler_load_kprocess("E", proc_e);

	/* Spin up proc_f process */
	kprint("Starting proc_f...\n");
	scheduler_load_kprocess("F", proc_f);

	/* Spin up proc_g process */
	kprint("Starting proc_g...\n");
	scheduler_load_kprocess("G", proc_g);

	/* Spin up proc_h process */
	kprint("Starting proc_h...\n");
	scheduler_load_kprocess("H", proc_h);

	/* Spin up proc_i process */
	kprint("Starting proc_i...\n");
	scheduler_load_kprocess("I", proc_i);

	/* Spin up proc_j process */
	kprint("Starting proc_j...\n");
	scheduler_load_kprocess("J", proc_j);

	/* Spin up proc_k process */
	kprint("Starting proc_k...\n");
	scheduler_load_kprocess("K", proc_k);

	/* Spin up proc_l process */
	kprint("Starting proc_l...\n");
	scheduler_load_kprocess("L", proc_l);

	/* Spin up proc_m process */
	kprint("Starting proc_m...\n");
	scheduler_load_kprocess("M", proc_m);

	/* Spin up proc_n process */
	kprint("Starting proc_n...\n");
	scheduler_load_kprocess("N", proc_n);
}
