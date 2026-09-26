; idt.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;

BITS 32

extern ex_fault_handler   ; exceptions.c
extern write_int_gate     ; idt.asm

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
    push gdt_selector_ke_code      ; selector
    push vec_dbz                   ; vector 
    call write_int_gate
    add esp, 12

    ; #UD
    push exc_ud                    ; offset
    push gdt_selector_ke_code      ; selector
    push vec_ud                    ; vector 
    call write_int_gate
    add esp, 12

    ; #DF
    push exc_df                    ; offset
    push gdt_selector_ke_code      ; selector
    push vec_df                    ; vector 
    call write_int_gate
    add esp, 12

    ; #TS
    push exc_ts                    ; offset
    push gdt_selector_ke_code      ; selector
    push vec_ts                    ; vector 
    call write_int_gate
    add esp, 12

    ; #NP
    push exc_np                    ; offset
    push gdt_selector_ke_code      ; selector
    push vec_np                    ; vector 
    call write_int_gate
    add esp, 12

    ; #SS
    push exc_ss                    ; offset
    push gdt_selector_ke_code      ; selector
    push vec_ss                    ; vector 
    call write_int_gate
    add esp, 12

    ; #GP
    push exc_gp                    ; offset
    push gdt_selector_ke_code      ; selector
    push vec_gp                    ; vector 
    call write_int_gate
    add esp, 12

    ; #PF
    push exc_pf                    ; offset
    push gdt_selector_ke_code      ; selector
    push vec_pf                    ; vector 
    call write_int_gate
    add esp, 12

    ret

; EXCEPTION HANDLERS

; esp+0  = vector
; esp+4  = error code
; esp+8  = eip
; esp+12 = cs
; esp+16 = eflags
exc_handler:    
    xchg edi, [esp+0]                  ; save edi; save vector in edi
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

    push esp                           ; exception state
    push edi                           ; exception vector
    call ex_fault_handler
    add esp, 8

    add esp, 18*4                      ; pop exception state

    push 0
    call kpanic
    add esp, 4

    iret

; esp+0 = eip
; esp+4 = cs
; esp+8 = eflags
exc_dbz:
    push 0                             ; fake error code
    push vec_dbz
    jmp exc_handler

; esp+0 = eip
; esp+4 = cs
; esp+8 = eflags
exc_ud:
    push 0                             ; fake error code
    push vec_ud
    jmp exc_handler

; esp+0  = error code
; esp+4  = eip
; esp+8  = cs
; esp+12 = eflags
exc_df:
    push vec_df
    jmp exc_handler

; esp+0  = error code
; esp+4  = eip
; esp+8  = cs
; esp+12 = eflags
exc_ts:
    push vec_ts
    jmp exc_handler

; esp+0  = error code
; esp+4  = eip
; esp+8  = cs
; esp+12 = eflags
exc_np:
    push vec_np
    jmp exc_handler

; esp+0  = error code
; esp+4  = eip
; esp+8  = cs
; esp+12 = eflags
exc_ss:
    push vec_ss
    jmp exc_handler

; esp+0  = error code
; esp+4  = eip
; esp+8  = cs
; esp+12 = eflags
exc_gp:
    push vec_gp
    jmp exc_handler

; esp+0  = error code
; esp+4  = eip
; esp+8  = cs
; esp+12 = eflags
exc_pf:
    push vec_pf
    jmp exc_handler
