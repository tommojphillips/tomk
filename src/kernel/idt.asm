BITS 32

extern exception_dbz   ; interrupts.c
extern exception_trap  ; interrupts.c
extern exception_nmi   ; interrupts.c
extern exception_int3  ; interrupts.c
extern exception_of    ; interrupts.c
extern exception_bound ; interrupts.c
extern exception_ud    ; interrupts.c
extern exception_df    ; interrupts.c
extern exception_ts    ; interrupts.c
extern exception_np    ; interrupts.c
extern exception_ss    ; interrupts.c
extern exception_gp    ; interrupts.c
extern exception_pf    ; interrupts.c
extern write_int_gate  ; idt.asm
extern kernel_hang     ; entry.asm
extern puts

global idt_init

%include "src\kernel\include\common.inc"

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

section .data

idt_descriptor:
    idt_desc_limit dw 0x3FF ; limit
    idt_desc_base  dd 0     ; base

section .text

idt_init:

    mov eax, [esp+4]
    mov [idt_desc_base], eax

    lidt [idt_descriptor]

    push KCODE                         ; selector
    push 10001110b                     ; access P=1 DPL=00 S=0 TYPE=1110 (INT 386)

    ; #DBZ
    push exc_dbz                   ; offset
    push vec_dbz                   ; vector 
    call write_int_gate
    add esp, 8

    ; #TRAP
    push exc_trap                  ; offset
    push vec_trap                  ; vector 
    call write_int_gate
    add esp, 8

    ; #NMI
    push exc_nmi                   ; offset
    push vec_nmi                   ; vector 
    call write_int_gate
    add esp, 8

    ; #INT3
    push exc_int3                  ; offset
    push vec_int3                  ; vector 
    call write_int_gate
    add esp, 8

    ; #OF
    push exc_of                    ; offset
    push vec_of                    ; vector 
    call write_int_gate
    add esp, 8

    ; #BOUND
    push exc_bound                 ; offset
    push vec_bound                 ; vector 
    call write_int_gate
    add esp, 8

    ; #UD
    push exc_ud                    ; offset
    push vec_ud                    ; vector 
    call write_int_gate
    add esp, 8

    ; #DF
    push exc_df                    ; offset
    push vec_df                    ; vector 
    call write_int_gate
    add esp, 8

    ; #TS
    push exc_ts                    ; offset
    push vec_ts                    ; vector 
    call write_int_gate
    add esp, 8

    ; #NP
    push exc_np                    ; offset
    push vec_np                    ; vector 
    call write_int_gate
    add esp, 8

    ; #SS
    push exc_ss                    ; offset
    push vec_ss                    ; vector 
    call write_int_gate
    add esp, 8

    ; #GP
    push exc_gp                    ; offset
    push vec_gp                    ; vector 
    call write_int_gate
    add esp, 8

    ; #PF
    push exc_pf                    ; offset
    push vec_pf                    ; vector 
    call write_int_gate
    add esp, 16

    sti                            ; enable interrupts
    
    ret

; EXCEPTION HANDLERS

exc_handler:    
    xchg edi, [esp+0]                   ; xchg ROUTINE and edi
    push esi
    push ebp
    push esp
    push ebx
    push edx
    push ecx
    push eax
    
    push gs
    push fs
    push ds
    push ss
    push cs
    push es

    push eax
    mov eax, cr3
    xchg eax, [esp+0]                   ; xchg cr3 and eax

    push eax
    mov eax, cr2
    xchg eax, [esp+0]                   ; xchg cr2 and eax

    push eax
    mov eax, cr0
    xchg eax, [esp+0]                   ; xchg cr0 and eax

    test edi, edi                      ; NULL?
    jz .skip                           ; yes. skip
;                                      ; no, call ROUTINE
    
    push esp
    call edi
    add esp, 4

.skip:
    add esp, 18*4                      ; pop STATE

    call kernel_hang
    iret

exc_dbz:
    push 0                            ; fake error code
    push exception_dbz
    jmp exc_handler

exc_trap:
    push 0                            ; fake error code
    push exception_trap
    jmp exc_handler

exc_nmi:
    push 0                            ; fake error code
    push exception_nmi
    jmp exc_handler

exc_int3:
    push 0                            ; fake error code
    push exception_int3
    jmp exc_handler

exc_of:
    push 0                            ; fake error code
    push exception_of
    jmp exc_handler

exc_bound:
    push 0                            ; fake error code
    push exception_bound
    jmp exc_handler

exc_ud:
    push 0                            ; fake error code
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
