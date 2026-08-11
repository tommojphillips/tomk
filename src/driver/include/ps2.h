/* ps2.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef DRIVER_PS2_H
#define DRIVER_PS2_H

extern void ps2_cpu_reset(void);
extern int ps2_sc2ch(int scancode);
extern int ps2_getchar(void);
extern int ps2_getscancode(void);

#endif
