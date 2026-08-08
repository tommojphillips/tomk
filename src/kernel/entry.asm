; entry.asm
BITS 32

extern printf                ; libc\stdio\printf.c
extern kernel_main           ; kernel.c

global multiboot_info_ptr
global kernel_entry
global kernel_hang

section .bss
stack_bottom:
    align 16, db 0
    stack_buf resb 16*1024   ; reserve 16kb for stack
stack_top:

section .rodata
    hang_str db "HANG @ %8.8X", 0

section .data
    multiboot_info_ptr dd 0    

section .text

kernel_entry:
    cmp eax, 0x2BADB002      ; multiboot ?
    jnz .unk                 ; no

.multiboot:                  ; yes
    mov [multiboot_info_ptr], ebx
    jmp .done
.unk:
    mov [multiboot_info_ptr], -1

.done:
    mov esp, stack_top       ; change stacks
    call kernel_main         ; call into the kernel

kernel_hang:
    push hang_str
    call printf
    add esp, 4
.hang:
    cli
    hlt
    jmp .hang
