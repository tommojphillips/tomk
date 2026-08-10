; entry.asm
BITS 32

extern kernel_main           ; kernel.c

global multiboot_info_ptr
global kernel_entry

section .bss
stack_bottom:
    align 16, db 0
    stack_buf resb 16*1024   ; reserve 16kb for stack
stack_top:

section .data
    multiboot_info_ptr dd 0    

section .text

kernel_entry:
    cli
    
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

.hang:
    cli
    hlt
    jmp .hang
