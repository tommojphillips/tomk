; tss.asm

BITS 32

extern write_tss_gate

global tss_init

%include "src\kernel\include\common.inc"

Section .bss
    align 8, db 0
    tss resb 104

Section .text

tss_init:

    ; Write TSS0
    push tss                ; base
    push 10001001b          ; access P=1 DPL=00 S=0 TYPE=1001 (386 Available TSS)
    push 0x68               ; limit
    push KTSS               ; selector 
    call write_tss_gate
    add esp, 16

    ; Load TR
    mov ax, KTSS            ; tss0
    ltr ax

    ret
