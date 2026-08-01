BITS 32
global tss_init

section .text

tss_init:
    ; Load TR
    mov bx, 0x38 ; tss0
    ltr bx

    ret