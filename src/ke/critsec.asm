; critsec.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; Critical section guards
;

BITS 32

global critsec_enter
global critsec_leave

; Enter critical section
; Returns 1 if IF was set, otherwise returns 0
critsec_enter:
    pushfd
    pop eax
    shr eax, 9
    and eax, 1
    cli
    ret

; Leave critical section
; esp+4 = context
critsec_leave:
    mov eax, [esp+4]
    test eax, 1
    jz .done
    sti
.done:
    ret
