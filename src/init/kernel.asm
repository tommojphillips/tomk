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

extern kmain                                     ; kernel.c
extern kprintf                                   ; kernel.c
extern kdprintf                                  ; kernel.c

extern sec_boot_start                            ; linker.ld
extern sec_boot_end                              ; linker.ld

extern pg_unmap                                  ; paging.asm
extern pg_invalidate                             ; paging.asm

extern khang                                     ; khang.asm

global kernel_init
global kstack_base
global kstack_top

%include "src\include\kspacedef.inc"
%include "src\mm\include\paging.inc"

section .bss
    align 16
kstack_base:
    resb 1024*1024
kstack_top:

section .text

; kernel init
kernel_init:

    ; setup stack
    mov esp, kstack_top
    xor ebp, ebp

    ; compute .boot_section size
    mov edx, sec_boot_end
    add edx, PAGE_SIZE-1
    sub edx, sec_boot_start
    shr edx, 12                                  ; page_count = (end + 0xFFF - start) >> 12

    ; ummap .boot section identity map
    push edx                                     ; page_count
    push sec_boot_start                          ; virtual address
    call pg_unmap                                ; ummap .boot section identity map
    add esp, 8
    
    call gdt_init                                ; setup gdt, reload segment registers
    call idt_init                                ; setup idt, write int handlers
    call tss_init                                ; setup tss, load tr
    call pic_init                                ; setup pic
    call pit_init                                ; setup pit
    call ps2_init                                ; setup ps/2

    push 0x3F8                                   ; COM1
    call serial_init                             ; setup uart
    add esp, 4

    sti                                          ; enable interrupts

    call kmain                                   ; call into the c entry point
    call khang                                   ; hang
