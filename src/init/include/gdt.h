/* gdt.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>

#ifndef _GDT_H
#define _GDT_H

extern const uint32_t* KCODE;  /* Kernel code selector */
extern const uint32_t* KDATA;  /* Kernel data selector */
extern const uint32_t* KSTACK; /* Kernel stack selector */

extern const uint32_t* UCODE;  /* User code selector */
extern const uint32_t* UDATA;  /* User data selector */
extern const uint32_t* USTACK; /* User stack selector */

#define KCODE  (((uint32_t)&KCODE) & 0xFFF8)
#define KDATA  (((uint32_t)&KDATA) & 0xFFF8)
#define KSTACK (((uint32_t)&KSTACK) & 0xFFF8)

#define UCODE  (((uint32_t)&UCODE) & 0xFFF8)
#define UDATA  (((uint32_t)&UDATA) & 0xFFF8)
#define USTACK (((uint32_t)&USTACK) & 0xFFF8)

#endif
