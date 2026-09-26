/* gdt.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <stdint.h>

#ifndef _GDT_H
#define _GDT_H

extern const uint32_t* gdt_selector_ke_code;  /* Kernel code selector */
extern const uint32_t* gdt_selector_ke_data;  /* Kernel data selector */
extern const uint32_t* gdt_selector_ke_stack; /* Kernel stack selector */

extern const uint32_t* gdt_selector_user_code;    /* User code selector */
extern const uint32_t* gdt_selector_user_data;    /* User data selector */
extern const uint32_t* gdt_selector_user_stack;   /* User stack selector */

#define KCODE  (((uint32_t)&gdt_selector_ke_code) & 0xFFF8)
#define KDATA  (((uint32_t)&gdt_selector_ke_data) & 0xFFF8)
#define KSTACK (((uint32_t)&gdt_selector_ke_stack) & 0xFFF8)

#define UCODE  (((uint32_t)&gdt_selector_user_code) & 0xFFF8)
#define UDATA  (((uint32_t)&gdt_selector_user_data) & 0xFFF8)
#define USTACK (((uint32_t)&gdt_selector_user_stack) & 0xFFF8)

#endif
