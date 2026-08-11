/* kernel.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KERNEL_H
#define KERNEL_H

void kernel_panic(const char* fmt, ...);
void kernel_hang(void);

#endif
