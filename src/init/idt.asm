; idt.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;

BITS 32

extern exception_dbz   ; exceptions.c
extern exception_trap  ; exceptions.c
extern exception_nmi   ; exceptions.c
extern exception_int3  ; exceptions.c
extern exception_of    ; exceptions.c
extern exception_bound ; exceptions.c
extern exception_ud    ; exceptions.c
extern exception_df    ; exceptions.c
extern exception_ts    ; exceptions.c
extern exception_np    ; exceptions.c
extern exception_ss    ; exceptions.c
extern exception_gp    ; exceptions.c
extern exception_pf    ; exceptions.c
extern write_int_gate  ; idt.asm
extern kpanic          ; kernel.asm

global idt_init
global idt

%include "src\include\common.inc"

vec_dbz     equ 0x00 ; Divide by zero interrupt
vec_trap    equ 0x01 ; Trap interrupt
vec_nmi     equ 0x02 ; Non-maskable interrupt
vec_int3    equ 0x03 ; Breakpoint interrupt
vec_of      equ 0x04 ; Overflow interrupt
vec_bound   equ 0x05 ; Bound interrupt
vec_ud      equ 0x06 ; Undefined exception
vec_df      equ 0x08 ; Double-fault exception
vec_ts      equ 0x0A ; Task-segment exception
vec_np      equ 0x0B ; Not-present exception
vec_ss      equ 0x0C ; Stack-segment exception
vec_gp      equ 0x0D ; General-protection exception
vec_pf      equ 0x0E ; Page-fault exception

Section .bss
    idt resb 8*128

Section .data
idt_descriptor:
    dw 0x3FF ; limit
    dd idt   ; base

Section .text

idt_init:

    lidt [idt_descriptor]
    
    ; #DBZ
    push exc_dbz                   ; offset
    push KCODE                     ; selector
    push vec_dbz                   ; vector 
    call write_int_gate
    add esp, 12

    ; #TRAP
    push exc_trap                  ; offset
    push KCODE                     ; selector
    push vec_trap                  ; vector 
    call write_int_gate
    add esp, 12

    ; #NMI
    push exc_nmi                   ; offset
    push KCODE                     ; selector
    push vec_nmi                   ; vector 
    call write_int_gate
    add esp, 12

    ; #INT3
    push exc_int3                  ; offset
    push KCODE                     ; selector
    push vec_int3                  ; vector 
    call write_int_gate
    add esp, 12

    ; #OF
    push exc_of                    ; offset
    push KCODE                     ; selector
    push vec_of                    ; vector 
    call write_int_gate
    add esp, 12

    ; #BOUND
    push exc_bound                 ; offset
    push KCODE                     ; selector
    push vec_bound                 ; vector 
    call write_int_gate
    add esp, 12

    ; #UD
    push exc_ud                    ; offset
    push KCODE                     ; selector
    push vec_ud                    ; vector 
    call write_int_gate
    add esp, 12

    ; #DF
    push exc_df                    ; offset
    push KCODE                     ; selector
    push vec_df                    ; vector 
    call write_int_gate
    add esp, 12

    ; #TS
    push exc_ts                    ; offset
    push KCODE                     ; selector
    push vec_ts                    ; vector 
    call write_int_gate
    add esp, 12

    ; #NP
    push exc_np                    ; offset
    push KCODE                     ; selector
    push vec_np                    ; vector 
    call write_int_gate
    add esp, 12

    ; #SS
    push exc_ss                    ; offset
    push KCODE                     ; selector
    push vec_ss                    ; vector 
    call write_int_gate
    add esp, 12

    ; #GP
    push exc_gp                    ; offset
    push KCODE                     ; selector
    push vec_gp                    ; vector 
    call write_int_gate
    add esp, 12

    ; #PF
    push exc_pf                    ; offset
    push KCODE                     ; selector
    push vec_pf                    ; vector 
    call write_int_gate
    add esp, 12

    ret

; EXCEPTION HANDLERS

exc_handler:    
    xchg edi, [esp+0]                  ; save edi; save routine in edi
    push esi                           ; save esi
    push ebp                           ; save ebp
    push esp                           ; save esp
    push ebx                           ; save ebx
    push edx                           ; save edx
    push ecx                           ; save ecx
    push eax                           ; save eax
    
    push gs                            ; save gs
    push fs                            ; save fs
    push ds                            ; save ds
    push ss                            ; save ss
    push cs                            ; save cs
    push es                            ; save es

    push eax
    mov eax, cr3
    xchg eax, [esp+0]                  ; save cr3

    push eax
    mov eax, cr2
    xchg eax, [esp+0]                  ; save cr2

    push eax
    mov eax, cr0
    xchg eax, [esp+0]                  ; save cr0

    pushfd                             ; save eflags
    push 0                             ; save eip

    test edi, edi                      ; routine == NULL?
    jz .skip                           ; yes; dont call routine
    
    push esp
    call edi
    add esp, 4

.skip:
    add esp, 20*4                      ; pop STATE

    push 0
    call kpanic
    add esp, 4

    iret

exc_dbz:
    push 0                             ; fake error code
    push exception_dbz
    jmp exc_handler

exc_trap:
    push 0                             ; fake error code
    push exception_trap
    jmp exc_handler

exc_nmi:
    push 0                             ; fake error code
    push exception_nmi
    jmp exc_handler

exc_int3:
    push 0                             ; fake error code
    push exception_int3
    jmp exc_handler

exc_of:
    push 0                             ; fake error code
    push exception_of
    jmp exc_handler

exc_bound:
    push 0                             ; fake error code
    push exception_bound
    jmp exc_handler

exc_ud:
    push 0                             ; fake error code
    push exception_ud
    jmp exc_handler

exc_df:
    push exception_df
    jmp exc_handler

exc_ts:
    push exception_ts
    jmp exc_handler

exc_np:
    push exception_np
    jmp exc_handler

exc_ss:
    push exception_ss
    jmp exc_handler

exc_gp:
    push exception_gp
    jmp exc_handler

exc_pf:
    push exception_pf
    jmp exc_handler
