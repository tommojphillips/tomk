; tss.asm

BITS 32

extern write_tss_gate

global tss_init

%include "src\kernel\include\common.inc"

section .text

tss_init:
    mov eax, [esp+4]        ; tss_base
    
    ; Write TSS0
    push eax                ; base
    push 10001001b          ; access P=1 DPL=00 S=0 TYPE=1001 (386 Available TSS)
    push 0x68               ; limit
    push KTSS               ; selector 
    call write_tss_gate
    add esp, 16

    ; Load TR
    mov ax, KTSS            ; tss0
    ltr ax

    ret
