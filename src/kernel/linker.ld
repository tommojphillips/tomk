OUTPUT_FORMAT(elf32-i386)
ENTRY(kernel_entry)

SECTIONS {
    . = 0x00200000;

    /* Read-only data */
    .rodata BLOCK(4K) : ALIGN(4K) {
     *(.multiboot)
     *(.rodata)
    }

    /* Read-write data (initialized) */
    .data BLOCK(4K) : ALIGN(4K) {
     *(.data)
    }

    /* Read-write data (uninitialized) */
    .bss (NOLOAD) : ALIGN(4K) {
     *(COMMON)
     *(.bss)
    }

    .text BLOCK(4K) : ALIGN(4K) {
     *(.text)
    }
}