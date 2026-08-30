; multiboot.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; Multiboot header
;

BITS 32

MB_MAGIC    equ 0x1BADB002
MB_4KB      equ (1 << 0)
MB_MMAP     equ (1 << 1)

MB_FLAGS    equ (MB_4KB | MB_MMAP)
MB_CHECKSUM equ (0 - (MB_MAGIC + MB_FLAGS))
 
section .multiboot
	align 4, db 0
	dd MB_MAGIC
	dd MB_FLAGS
	dd MB_CHECKSUM
