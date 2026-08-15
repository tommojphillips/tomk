; kernel.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;

BITS 32

extern gdt_init
extern idt_init
extern tss_init
extern kernel_main
extern printf

global kernel_init
global kernel_hang
global kernel_panic

Section .bss
    ; reserve 16kb for stack
    align 16, db 0
    stack_bottom:
    resb 16*1024
    stack_top:

Section .rodata
    hang_str db "FATAL: HANG at EIP 0x%8.8X", 0
    panic_str db "FATAL: KERNEL PANIC at EIP 0x%8.8X", 10, 0

Section .text

; kernel init
kernel_init:
    mov esp, stack_top       ; setup stack

    call gdt_init
    call idt_init
    call tss_init

    call kernel_main         ; call into the c entry point
    call kernel_hang         ; hang

; Kernel hang; hang system indefinitely
; DOES NOT RETURN!
kernel_hang:
    push hang_str            ; print hang msg
    call printf
    add esp, 4

_hang:
    cli
    hlt
    jmp _hang

; Kernel panic; hang system indefinitely
; DOES NOT RETURN!
kernel_panic:
    
    mov eax, [esp+4]         ; msg
    test eax, eax            ; NULL?
    jz .pr_msg               ; yes, skip printing msg

    push eax                 ; print msg
    call printf
    add esp, 4

.pr_msg:
    push panic_str           ; print panic msg
    call printf
    add esp, 4

    jmp _hang                ; hang indefinitely
