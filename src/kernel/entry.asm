BITS 32

extern mm_init               ; mm.asm
extern gdt_init              ; gdt.asm
extern idt_init              ; idt.asm
extern exceptions_init       ; exceptions.asm
extern paging_init           ; paging.asm
extern tss_init              ; tss.asm
extern puts                  ; libc\stdio\puts.c

extern tty_init              ; driver\tty.c
extern pic_init              ; driver\pic.asm
extern ps2_init              ; driver\ps2.asm

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
    hang_str db "HANG", 0

section .data
    multiboot_info_ptr dd 0

section .text

kernel_entry:
    cmp eax, 0x2BADB002      ; multiboot ?
    jnz .unk                 ; no
    jmp .multiboot           ; yes

.multiboot:
    mov [multiboot_info_ptr], ebx
    jmp .done

.unk:
    mov [multiboot_info_ptr], -1
    jmp .done

.done:
    call gdt_init            ; setup gdt
    call tty_init            ; setup tty
    call idt_init            ; setup idt
    call paging_init         ; setup paging
    call tss_init            ; setup tss
    call pic_init            ; setup pic
    call ps2_init            ; setup ps/2
    
    call kernel_main         ; call into the kernel

kernel_hang:
    push hang_str
    call puts
    add esp, 4
.hang:
    cli
    hlt
    jmp .hang
