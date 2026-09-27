; idt.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;

BITS 32

extern ex_fault_handler                          ; exceptions.c
extern write_int_gate                            ; i86.asm

global idt_init
global idt                                       ; idt

%include "src\include\common.inc"

vec_de      equ 0x00                             ; Fault - Division error
vec_db      equ 0x01                             ; Trap  - Debug
vec_bp      equ 0x03                             ; Trap  - Breakpoint
vec_of      equ 0x04                             ; Trap  - Overflow
vec_br      equ 0x05                             ; Fault - Bound range exceeeded
vec_ud      equ 0x06                             ; Fault - Invalid opcode
vec_df      equ 0x08                             ; Abort - Double fault
vec_ts      equ 0x0A                             ; Fault - Invalid TSS
vec_np      equ 0x0B                             ; Fault - Segment not present
vec_ss      equ 0x0C                             ; Fault - Stack segment fault
vec_gp      equ 0x0D                             ; Fault - General protection fault
vec_pf      equ 0x0E                             ; Fault - Page fault

Section .bss
    idt resb 8*128

Section .data
idt_descriptor:
    dw 0x3FF                                     ; limit
    dd idt                                       ; base

Section .text

idt_init:

    lidt [idt_descriptor]
    
    ; #DE
    push exc_de                                   ; offset
    push gdt_selector_ke_code                     ; selector
    push vec_de                                   ; vector 
    call write_int_gate
    add esp, 12

    ; #UD
    push exc_ud                                  ; offset
    push gdt_selector_ke_code                    ; selector
    push vec_ud                                  ; vector 
    call write_int_gate
    add esp, 12

    ; #DF
    push exc_df                                  ; offset
    push gdt_selector_ke_code                    ; selector
    push vec_df                                  ; vector 
    call write_int_gate
    add esp, 12

    ; #TS
    push exc_ts                                  ; offset
    push gdt_selector_ke_code                    ; selector
    push vec_ts                                  ; vector 
    call write_int_gate
    add esp, 12

    ; #NP
    push exc_np                                  ; offset
    push gdt_selector_ke_code                    ; selector
    push vec_np                                  ; vector 
    call write_int_gate
    add esp, 12

    ; #SS
    push exc_ss                                  ; offset
    push gdt_selector_ke_code                    ; selector
    push vec_ss                                  ; vector 
    call write_int_gate
    add esp, 12

    ; #GP
    push exc_gp                                  ; offset
    push gdt_selector_ke_code                    ; selector
    push vec_gp                                  ; vector 
    call write_int_gate
    add esp, 12

    ; #PF
    push exc_pf                                  ; offset
    push gdt_selector_ke_code                    ; selector
    push vec_pf                                  ; vector 
    call write_int_gate
    add esp, 12

    ret

; Common fault handler
; esp+0  = vector
; esp+4  = error code
; esp+8  = eip
; esp+12 = cs
; esp+16 = eflags
; esp+20 = esp2 ( if priv change )
; esp+24 = ss2  ( if priv change )
exc_handler:
    xchg edi, [esp+0]                            ; edi <-> *vector 
    push esi                                     ; save esi
    push ebp                                     ; save ebp
    push esp                                     ; save esp
    push ebx                                     ; save ebx
    push edx                                     ; save edx
    push ecx                                     ; save ecx
    push eax                                     ; save eax
    
    push gs                                      ; save gs
    push fs                                      ; save fs
    push ds                                      ; save ds
    push ss                                      ; save ss
    push es                                      ; save es

    push eax
    mov eax, cr3
    xchg eax, [esp+0]                            ; save cr3

    push eax
    mov eax, cr2
    xchg eax, [esp+0]                            ; save cr2

    push eax
    mov eax, cr0
    xchg eax, [esp+0]                            ; save cr0

    push esp                                     ; state
    push edi                                     ; vector
    call ex_fault_handler
    add esp, 8

    add esp, 18*4                                ; pop state + frame
    
    hlt

    iret

; Division fault handler
; esp+0  = eip
; esp+4  = cs
; esp+8  = eflags
; esp+12 = esp2 ( if priv change )
; esp+16 = ss2  ( if priv change )
exc_de:
    push 0                                       ; normalize exception frame
    push vec_de
    jmp exc_handler

; Invalid opcode fault handler
; esp+0  = eip
; esp+4  = cs
; esp+8  = eflags
; esp+12 = esp2 ( if priv change )
; esp+16 = ss2  ( if priv change )
exc_ud:
    push 0                                       ; normalize exception frame
    push vec_ud
    jmp exc_handler

; Double fault handler
; esp+0  = error code (zero)
; esp+4  = eip
; esp+8  = cs
; esp+12 = eflags
; esp+16 = esp2 ( if priv change )
; esp+20 = ss2  ( if priv change )
exc_df:
    push vec_df
    jmp exc_handler

; Invalid TSS fault handler
; esp+0  = error code
; esp+4  = eip
; esp+8  = cs
; esp+12 = eflags
; esp+16 = esp2 ( if priv change )
; esp+20 = ss2  ( if priv change )
exc_ts:
    push vec_ts
    jmp exc_handler

; Segment not present fault handler
; esp+0  = error code
; esp+4  = eip
; esp+8  = cs
; esp+12 = eflags
; esp+16 = esp2 ( if priv change )
; esp+20 = ss2  ( if priv change )
exc_np:
    push vec_np
    jmp exc_handler

; Stack segment fault handler
; esp+0  = error code
; esp+4  = eip
; esp+8  = cs
; esp+12 = eflags
; esp+16 = esp2 ( if priv change )
; esp+20 = ss2  ( if priv change )
exc_ss:
    push vec_ss
    jmp exc_handler

; General protection fault handler
; esp+0  = error code
; esp+4  = eip
; esp+8  = cs
; esp+12 = eflags
; esp+16 = esp2 ( if priv change )
; esp+20 = ss2  ( if priv change )
exc_gp:
    push vec_gp
    jmp exc_handler

; Page fault handler
; esp+0  = error code
; esp+4  = eip
; esp+8  = cs
; esp+12 = eflags
; esp+16 = esp2 ( if priv change )
; esp+20 = ss2  ( if priv change )
exc_pf:
    push vec_pf
    jmp exc_handler
