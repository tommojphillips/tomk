; lock.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; spin locks
;

BITS 32

global spinlock_acquire
global spinlock_release

; Aquire lock
; esp+4 = lock pointer
spinlock_acquire:
    push edx
    mov edx, [esp+4+4]                           ; lock pointer    
    test edx, edx                                ; lock == NULL?
    jz .done
.spin:
    mov eax, 0                                   ; expecting unlocked (0)
    mov ecx, 1                                   ; set locked (1)
    lock cmpxchg [edx], ecx                      ; if (lock->locked == 0) lock->locked = 1
    jnz .spin                                    ; spin until lock->locked == 0
.done:
    pop edx
    ret

; Release lock
; esp+4 = lock pointer
spinlock_release:
    mov eax, [esp+4]                             ; lock pointer    
    test eax, eax                                ; lock == NULL?
    jz .done
    mov dword [eax], 0                           ; lock->locked = 0
.done:
    ret
