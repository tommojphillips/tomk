; critsec.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; Critical section guards
;

BITS 32

global critsec_enter
global critsec_leave

; Enter critical section
critsec_enter:
    cli
    ret

; Leave critical section
critsec_leave:
    sti
    ret
