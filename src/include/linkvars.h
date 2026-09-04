/* linkvars.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 * Linker section variables.
 * Use (size_t)&<sec> to get the address of section start and end.
 */

#ifndef LINK_VARS_H
#define LINK_VARS_H

extern const void* sec_mb_start;     /* linker.ld */
extern const void* sec_mb_end;       /* linker.ld */

extern const void* sec_text_start;   /* linker.ld */
extern const void* sec_text_end;     /* linker.ld */

extern const void* sec_boot_start;   /* linker.ld */
extern const void* sec_boot_end;     /* linker.ld */

extern const void* sec_rodata_start; /* linker.ld */
extern const void* sec_rodata_end;   /* linker.ld */

extern const void* sec_data_start;   /* linker.ld */
extern const void* sec_data_end;     /* linker.ld */

extern const void* sec_bss_start;    /* linker.ld */
extern const void* sec_bss_end;      /* linker.ld */

extern const void* sec_kstart;       /* linker.ld */
extern const void* sec_kend;         /* linker.ld */

#endif
