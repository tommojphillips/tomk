/* linker.ld
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#include <kspacedef.h>

ENTRY(_start)

SECTIONS {
    . = KPHYS;
    sec_kstart = .;
    
    /* TOMK Loader is executed at KPHYS */
    .boot : {
     sec_boot_start = .;
     *(.mb)
     *(.boot)
     sec_boot_end = .;
    }

    /* Kernel is executed at KVIRT */
    . += KVIRT;

    /* TOMK Kernel is executed at KVIRT */
    .text ALIGN(4K) : AT(V2P(ADDR(.text))) {
     sec_text_start = .;
     *(.text)
     sec_text_end = .;
    }

    .rodata ALIGN(4K) : AT(V2P(ADDR(.rodata))) {
     sec_rodata_start = .;
     *(.rodata)
     sec_rodata_end = .;
    }

    .data ALIGN(4K) : AT(V2P(ADDR(.data))) {
     sec_data_start = .;
     *(.data)
     sec_data_end = .;
    }

    .bss ALIGN(4K) : AT(V2P(ADDR(.bss))) {
     sec_bss_start = .;
     *(COMMON)
     *(.bss)
     sec_bss_end = .;
    }

    sec_kend = .;
}
