; cswitch.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; Context switching

BITS 32

; Debugging
;%define CSWITCH_DBG 1

extern scheduler_switch      ; scheduler.c
extern pit_send_eoi          ; pit.asm

%ifdef CSWITCH_DBG
extern scheduler_debug_cswitch_save
extern scheduler_debug_cswitch_load
%endif

global cswitch_handler

; Interrupt frame struct
struc FRAME
    ; PUSHA
    .edi    resd 1
    .esi    resd 1
    .ebp    resd 1
    .esp    resd 1
    .ebx    resd 1
    .edx    resd 1
    .ecx    resd 1
    .eax    resd 1
    
    ; INT frame
    .eip    resd 1
    .cs     resd 1
    .eflags resd 1

    ; Ring3 INT frame (only present when interrupted from Ring3)
    .esp2   resd 1
    .ss2    resd 1
endstruc

; Process context struct
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
    sub esp, 8

    ; Send EOI
    call pit_send_eoi

    ; Create storage for 2 pointers
    lea eax, [ebp-4]                             ; current proc
    lea edx, [ebp-8]                             ; next proc

    ; scheduler_switch(&current, &next)
    push edx                                     ; next proc
    push eax                                     ; current proc
    call scheduler_switch
    add esp, 8
    
    test eax, eax
    jnz .switch

.restore:
    ; Restore current proc
    add esp, 8                                   ; cleanup the 2 pointers
    popa
    iret

.switch:
    ; Save current proc
    push ebp                                     ; interrupt frame pointer
    push [ebp-4]                                 ; current proc
    call cswitch_save
    add esp, 8

    ; Load next proc
    push [ebp-8]                                 ; next proc
    call cswitch_load
    ; !! DOES NOT RETURN !!

; Save context
; ebp+8  = process context pointer
; ebp+12 = interrupt frame pointer
cswitch_save:
    push ebp
    mov ebp, esp

    push esi
    push edi

    mov edi, [ebp+8]                             ; process context pointer
    mov esi, [ebp+12]                            ; interrupt frame pointer

    ; context == NULL?
    test edi, edi
    jz .done

    ; Save interrupt frame 
    mov eax, [esi+FRAME.eflags]
    mov [edi+CTX.eflags], eax

    mov eax, [esi+FRAME.cs]
    mov [edi+CTX.cs], eax

    mov eax, [esi+FRAME.eip]
    mov [edi+CTX.eip], eax
    
    ; User mode switch?
    test [esi+FRAME.cs], 3
    jnz .user

.kernel:
    ; Save kernel stack
    mov ax, ss
    mov [edi+CTX.ss], ax

    mov eax, [esi+FRAME.esp]
    add eax, 12
    mov [edi+CTX.esp], eax
    jmp .save

.user:
    ; Save user stack
    mov ax, [esi+FRAME.ss2]
    mov [edi+CTX.ss], ax

    mov eax, [esi+FRAME.esp2]
    mov [edi+CTX.esp], ax

.save:
    ; Save general registers
    mov eax, [esi+FRAME.eax]
    mov [edi+CTX.eax], eax

    mov eax, [esi+FRAME.ecx]
    mov [edi+CTX.ecx], eax

    mov eax, [esi+FRAME.edx]
    mov [edi+CTX.edx], eax

    mov eax, [esi+FRAME.ebx]
    mov [edi+CTX.ebx], eax

    mov eax, [esi+FRAME.ebp]
    mov [edi+CTX.ebp], eax

    mov eax, [esi+FRAME.esi]
    mov [edi+CTX.esi], eax

    mov eax, [esi+FRAME.edi]
    mov [edi+CTX.edi], eax

    ; Save data segment registers
    mov ax, es
    mov [edi+CTX.es], ax
    mov ax, ds
    mov [edi+CTX.ds], ax
    mov ax, fs
    mov [edi+CTX.fs], ax
    mov ax, gs
    mov [edi+CTX.gs], ax
    
    ; Save cr3
    mov eax, cr3
    mov [edi+CTX.cr3], eax

%ifdef CSWITCH_DBG   
    ; Debug
    push edi                                     ; context
    call scheduler_debug_cswitch_save
    add esp, 4
%endif

.done:
    pop edi
    pop esi
    leave
    ret

; Context switch
; esp+4 = process context pointer
; !! DOES NOT RETURN !!
cswitch_load:
    mov edi, [esp+4]                             ; context

%ifdef CSWITCH_DBG
    ; Debug
    push edi                                     ; context
    call scheduler_debug_cswitch_load
    add esp, 4
%endif

    ; Load cr3
    mov eax, [edi+CTX.cr3]
    mov cr3, eax

    ; User mode switch?
    test [edi+CTX.cs], 3
    jnz .user

.kernel:
    ; Load kernel stack
    mov ax, [edi+CTX.ss]
    mov ss, ax
    mov esp, [edi+CTX.esp]
    jmp .load

.user:
    ; Build privilege level transition interrupt frame
    push [edi+CTX.ss]
    push [edi+CTX.esp]

.load:
    ; Build interrupt frame
    push [edi+CTX.eflags]
    push [edi+CTX.cs]
    push [edi+CTX.eip]

    ; Load data segment registers
    mov ax, [edi+CTX.es]
    mov es, ax
    mov ax, [edi+CTX.ds]
    mov ds, ax
    mov ax, [edi+CTX.fs]
    mov fs, ax
    mov ax, [edi+CTX.gs]
    mov gs, ax

    ; Load general registers
    mov eax, [edi+CTX.eax]
    mov ecx, [edi+CTX.ecx]
    mov edx, [edi+CTX.edx]
    mov ebx, [edi+CTX.ebx]
    mov ebp, [edi+CTX.ebp]
    mov esi, [edi+CTX.esi]
    mov edi, [edi+CTX.edi]

    ; Load eip, cs, eflags, (user mode transition: ss, esp)
    iret
