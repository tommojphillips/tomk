; multiboot.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; Multiboot header
;

BITS 32

MB_MAGIC    equ 0x1BADB002
MB_FLAGS    equ ((1 << 0) | (1 << 1) | (1 << 5) | (1 << 6))
MB_CHECKSUM equ (0 - (MB_MAGIC + MB_FLAGS))
 
section .multiboot
	align 4, db 0
	dd MB_MAGIC
	dd MB_FLAGS
	dd MB_CHECKSUM
