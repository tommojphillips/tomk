OUTPUT_FORMAT(elf32-i386)
ENTRY(kernel_entry)

SECTIONS {
    . = 0x00100000;

    . = ALIGN(4K);
    kstart = .;

    . = ALIGN(4K);
    sec_multiboot_start = .;

    /* Read-only data */
    .multiboot BLOCK(4K) : ALIGN(4K) {
     *(.multiboot)
    }

    . = ALIGN(4K);
    sec_multiboot_end = .;

    . = ALIGN(4K);
    sec_text_start = .;

    /* Code */
    .text BLOCK(4K) : ALIGN(4K) {
     *(.text)
    }

    . = ALIGN(4K);
    sec_text_end = .;

    . = ALIGN(4K);
    sec_rodata_start = .;

    /* Read-only data */
    .rodata BLOCK(4K) : ALIGN(4K) {
     *(.rodata)
    }

    . = ALIGN(4K);
    sec_rodata_end = .;

    . = ALIGN(4K);
    sec_data_start = .;

    /* Read-write data (initialized) */
    .data BLOCK(4K) : ALIGN(4K) {
     *(.data)
    }

    . = ALIGN(4K);
    sec_data_end = .;

    . = ALIGN(4K);
    sec_bss_start = .;

    /* Read-write data (uninitialized) */
    .bss (NOLOAD) : ALIGN(4K) {
     *(COMMON)
     *(.bss)
    }

    . = ALIGN(4K);
    sec_bss_end = .;

    . = ALIGN(4K);
    kend = .;
}
