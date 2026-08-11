; entry.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; Kernel Entry
;

BITS 32

extern kernel_init           ; kernel.asm

global multiboot_info_ptr
global kernel_entry

section .data
    multiboot_info_ptr dd 0    

section .text

kernel_entry:
    cli

.chk_bldr:
    cmp eax, 0x2BADB002      ; multiboot ?
    jnz .unk                 ; no

.mb:
    mov [multiboot_info_ptr], ebx
    jmp .done

.unk:
    mov [multiboot_info_ptr], -1

.done:
    jmp kernel_init
