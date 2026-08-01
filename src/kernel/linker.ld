OUTPUT_FORMAT(elf32-i386)
ENTRY(kernel_entry)

SECTIONS {
    . = 0x00200000;

    .first_section BLOCK(4K) : ALIGN(4K) {
     kstart = .;
    }

    /* Read-only data */
    .multiboot BLOCK(4K) : ALIGN(4K) {
     kstart_multiboot = .;
     *(.multiboot)
    }
    .multiboot_end BLOCK(4K) : ALIGN(4K) {
     kend_multiboot = .;
    }

    /* Code */
    .text BLOCK(4K) : ALIGN(4K) {
     kstart_text = .;
     *(.text)
    }
    .text_end BLOCK(4K) : ALIGN(4K) {
     kend_text = .;
    }

    /* Read-only data */
    .rodata BLOCK(4K) : ALIGN(4K) {
     kstart_rodata = .;
     *(.rodata)
    }
    .rodata_end BLOCK(4K) : ALIGN(4K) {
     kend_rodata = .;
    }

    /* Read-write data (initialized) */
    .data BLOCK(4K) : ALIGN(4K) {
     kstart_data = .;
     *(.data)
    }
    .data_end BLOCK(4K) : ALIGN(4K) {
     kend_data = .;
    }

    /* Read-write data (uninitialized) */
    .bss (NOLOAD) : ALIGN(4K) {
     kstart_bss = .;
     *(COMMON)
     *(.bss)
    }
    .bss_end (NOLOAD) : ALIGN(4K) {
     kend_bss = .;
    }

    .last_section (NOLOAD) : ALIGN(4K) {
     kend = .;
    }
}
