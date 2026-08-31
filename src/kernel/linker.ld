OUTPUT_FORMAT(elf32-i386)
ENTRY(_start) /* entry.asm */

SECTIONS {
    /* Load kernel at physical address 0x00100000 (1MB) */
    . = 0x00100000;
    sec_kstart = .;
    
    /* Loader is executed at 0x00100000 */
    .boot : {
     sec_boot_start = .;
     *(.mb)
     *(.boot)
     sec_boot_end = .;
    }

    /* Kernel is executed at 0xC0100000 */
    . += 0xC0000000;

    .text ALIGN (4K) : AT (ADDR (.text) - 0xC0000000) {
     sec_text_start = .;
     *(.text)
     sec_text_end = .;
    }

    .rodata ALIGN (4K) : AT (ADDR (.rodata) - 0xC0000000) {
     sec_rodata_start = .;
     *(.rodata)
     sec_rodata_end = .;
    }

    .data ALIGN (4K) : AT (ADDR (.data) - 0xC0000000) {
     sec_data_start = .;
     *(.data)
     sec_data_end = .;
    }

    .bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000) {
     sec_bss_start = .;
     *(COMMON)
     *(.bss)
     sec_bss_end = .;
    }

    sec_kend = .;
}
