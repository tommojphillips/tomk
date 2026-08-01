BITS 32

MB_MAGIC    equ 0x1BADB002
MB_FLAGS    equ (0x1 | 0x2)
MB_CHECKSUM equ (0 - (MB_MAGIC + MB_FLAGS))
 
section .multiboot
	align 4, db 0
	dd MB_MAGIC
	dd MB_FLAGS
	dd MB_CHECKSUM