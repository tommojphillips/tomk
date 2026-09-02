; kernel.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;

BITS 32

extern gdt_init                                  ; gdt.asm
extern idt_init                                  ; idt.asm
extern tss_init                                  ; tss.asm

extern pic_init                                  ; driver\pic\pic.asm
extern pit_init                                  ; driver\pit\pit.asm
extern ps2_init                                  ; driver\ps2\ps2.asm
extern serial_init                               ; driver\uart\uart.asm

extern kernel_main                               ; kernel.c
extern kprintf                                   ; kernel.c
extern kdprintf                                  ; kernel.c

extern sec_boot_start                            ; linker.ld
extern sec_boot_end                              ; linker.ld

extern pg_unmap                                  ; paging.asm
extern pg_invalidate                             ; paging.asm

global kernel_init
global kernel_hang
global kstack_base
global kstack_top

%include "src\kernel\include\common.inc"
%include "src\kernel\include\paging.inc"

section .bss
    align 16
kstack_base:
    resb 1024*1024
kstack_top:
section .rodata
    hang_str db "FATAL: HANG at EIP 0x%8.8X", 0

section .text

; kernel init
kernel_init:
    mov esp, kstack_top                          ; setup stack
    
    ; compute .boot_section size
    mov edx, sec_boot_end
    add edx, 0xFFF
    sub edx, sec_boot_start
    shr edx, 12                                  ; page_count = (end + 0xFFF - start) >> 12

    ; ummap .boot section identity map
    push edx                                     ; page_count
    push sec_boot_start                          ; virtual address
    call pg_unmap                                ; ummap .boot section identity map
    add esp, 8
    
    ; invalidate .boot section identity map
    push edx                                     ; page_count
    push sec_boot_start                          ; virtual address
    call pg_invalidate                           ; invalidate .boot section identity map
    add esp, 8
    
    call gdt_init                                ; setup gdt
    call idt_init                                ; setup idt, int handlers
    call tss_init                                ; setup tss
    call pic_init                                ; setup pic
    call pit_init                                ; setup pit
    call ps2_init                                ; setup ps/2
    call serial_init                             ; setup uart

    sti                                          ; enable interrupts

    call kernel_main                             ; call into the c entry point
    call kernel_hang                             ; hang

; Kernel hang; hang system indefinitely
; DOES NOT RETURN!
kernel_hang:
    dec [esp+0]
    push hang_str                                ; print hang msg
    call kdprintf
    add esp, 4

.hang:
    cli
    hlt
    jmp .hang
