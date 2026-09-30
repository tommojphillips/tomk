; khang.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;

BITS 32

global khang
extern kdprintf

Section .rodata
    hang_str db "FATAL: HANG at EIP 0x%8.8X", 0
    
Section .text

; Kernel hang; hang system indefinitely
; DOES NOT RETURN!
khang:
    dec [esp+0]
    push hang_str                                ; print hang msg
    call kdprintf
    add esp, 4

.hang:
    cli
    hlt
    jmp .hang
