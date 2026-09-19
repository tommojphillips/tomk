; cswitch.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; Context switching

BITS 32

extern scheduler_switch      ; scheduler.c
extern pic_send_eoi          ; pic.asm

global cswitch_handler

; interrupt frame struct
struc FRAME
    .edi    resd 1
    .esi    resd 1
    .ebp    resd 1
    .esp    resd 1
    .ebx    resd 1
    .edx    resd 1
    .ecx    resd 1
    .eax    resd 1

    .eip    resd 1
    .cs     resd 1
    .eflags resd 1
endstruc

; context switch struct
struc CTX
    .cr3    resd 1

    .gs     resd 1
    .fs     resd 1
    .ds     resd 1
    .ss     resd 1
    .es     resd 1

    .edi    resd 1
    .esi    resd 1
    .ebp    resd 1
    .esp    resd 1
    .ebx    resd 1
    .edx    resd 1
    .ecx    resd 1
    .eax    resd 1

    .eip    resd 1
    .cs     resd 1
    .eflags resd 1
endstruc

section .text

; Context switch interrupt handler
; esp+0 = eip
; esp+4 = cs
; esp+8 = eflags
cswitch_handler:
    pusha
    mov ebp, esp

    ; send EOI
    push 0
    call pic_send_eoi
    add esp, 4

    ; scheduler_switch(&current, &next)
    ; create storage for 2 pointers
    sub esp, 8
    
    lea eax, [esp+0]                             ; &current
    lea edx, [esp+4]                             ; &next

    push edx                                     ; &next
    push eax                                     ; &current
    call scheduler_switch
    add esp, 8
    
    test eax, eax
    jnz .switch

.restore:
    add esp, 8
    popa    
    iret

.switch:
    ; save current proc
    push [esp+0]                                 ; &current
    call cswitch_save
    add esp, 4

    ; load next proc
    push [esp+4]                                 ; &next
    call cswitch_load
    add esp, 4

; Save context
; esp+4 = context
cswitch_save:
    mov edi, [esp+4]

    ; save interrupt frame 
    mov eax, [ebp+FRAME.eflags]
    mov [edi+CTX.eflags], eax

    mov eax, [ebp+FRAME.cs]
    mov [edi+CTX.cs], eax

    mov eax, [ebp+FRAME.eip]
    mov [edi+CTX.eip], eax
    
    ; save general registers
    mov eax, [ebp+FRAME.eax]
    mov [edi+CTX.eax], eax

    mov eax, [ebp+FRAME.ecx]
    mov [edi+CTX.ecx], eax

    mov eax, [ebp+FRAME.edx]
    mov [edi+CTX.edx], eax

    mov eax, [ebp+FRAME.ebx]
    mov [edi+CTX.ebx], eax

    mov eax, [ebp+FRAME.ebp]
    mov [edi+CTX.ebp], eax

    mov eax, [ebp+FRAME.esp]
    add eax, 12
    mov [edi+CTX.esp], eax

    mov eax, [ebp+FRAME.esi]
    mov [edi+CTX.esi], eax

    mov eax, [ebp+FRAME.edi]
    mov [edi+CTX.edi], eax

    ; save segment registers
    mov ax, ss
    mov [edi+CTX.ss], ax
    mov ax, es
    mov [edi+CTX.es], ax
    mov ax, ds
    mov [edi+CTX.ds], ax
    mov ax, fs
    mov [edi+CTX.fs], ax
    mov ax, gs
    mov [edi+CTX.gs], ax
    
    ; save cr3
    mov eax, cr3
    mov [edi+CTX.cr3], eax

    ret

; Context switch
; esp+4 = context
cswitch_load:
    mov edi, [esp+4]

    ; load cr3
    mov eax, [edi+CTX.cr3]
    mov cr3, eax

    ; load stack
    mov ax, [edi+CTX.ss]
    mov ss, ax
    mov esp, [edi+CTX.esp]

    ; build interrupt frame 
    push [edi+CTX.eflags]
    push [edi+CTX.cs]
    push [edi+CTX.eip]

    ; load segment registers
    mov ax, [edi+CTX.es]
    mov es, ax
    mov ax, [edi+CTX.ds]
    mov ds, ax
    mov ax, [edi+CTX.fs]
    mov fs, ax
    mov ax, [edi+CTX.gs]
    mov gs, ax

    ; load general registers
    mov eax, [edi+CTX.eax]
    mov ecx, [edi+CTX.ecx]
    mov edx, [edi+CTX.edx]
    mov ebx, [edi+CTX.ebx]
    mov ebp, [edi+CTX.ebp]
    mov esi, [edi+CTX.esi]
    mov edi, [edi+CTX.edi]

    ; load eip, cs, eflags
    iret
