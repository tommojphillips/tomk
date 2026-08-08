; i86.asm

BITS 32

extern idt_base
extern gdt_base

global setregs
global getregs
global int86
global inb
global outb
global inw
global outw
global ind
global outd
global write_int_gate
global write_tss_gate
global setcr0
global setcr2
global setcr3

struc REGS
    .edi    resd 1
    .esi    resd 1
    .ebp    resd 1
    .esp    resd 1
    .ebx    resd 1
    .edx    resd 1
    .ecx    resd 1
    .eax    resd 1
    .es     resd 1
    .cs     resd 1
    .ss     resd 1
    .ds     resd 1
    .fs     resd 1
    .gs     resd 1
    .eflags resd 1
endstruc

REGS.ax equ REGS.eax
REGS.al equ REGS.eax
REGS.ah equ REGS.eax + 1

REGS.cx equ REGS.ecx
REGS.cl equ REGS.ecx
REGS.ch equ REGS.ecx + 1

REGS.dx equ REGS.edx
REGS.dl equ REGS.edx
REGS.dh equ REGS.edx + 1

REGS.bx equ REGS.ebx
REGS.bl equ REGS.ebx
REGS.bh equ REGS.ebx + 1

REGS.sp equ REGS.esp
REGS.bp equ REGS.ebp
REGS.si equ REGS.esi
REGS.di equ REGS.edi

; Set CPU Registers
; 2nd dword on stack is input registers
setregs:
    mov eax, [esp + 4]                 ; input_regs 
    test eax, eax                      ; is input_regs NULL?
    jz setregs_skip                    ; yes, skip assigning.
    mov edi, [eax + REGS.edi]          ; no, assign it.
    mov esi, [eax + REGS.esi]
    mov ebp, [eax + REGS.ebp]
    mov ebx, [eax + REGS.ebx]
    mov edx, [eax + REGS.edx]
    mov ecx, [eax + REGS.ecx]
    mov eax, [eax + REGS.eax]
setregs_skip:
    ret

; Get CPU Registers
; 2rd dword on stack is output registers
getregs:
    pusha                              ; save register state
    mov ebp, [esp + 36]                ; output_regs (+32 from pusha)
    test ebp, ebp                      ; output_regs == NULL
    jz getregs_skip                    ; yes, skip
                                       ; no, write it.

    mov eax, [esp + 0]
    mov [ebp + REGS.edi], eax

    mov eax, [esp + 4]
    mov [ebp + REGS.esi], eax

    mov eax, [esp + 8]
    mov [ebp + REGS.ebp], eax

    mov eax, [esp + 12]
    mov [ebp + REGS.esp], eax

    mov eax, [esp + 16]
    mov [ebp + REGS.ebx], eax

    mov eax, [esp + 20]
    mov [ebp + REGS.edx], eax

    mov eax, [esp + 24]
    mov [ebp + REGS.ecx], eax

    mov eax, [esp + 28]
    mov [ebp + REGS.eax], eax

getregs_skip:
    popa                               ; restore regiser state
    ret

; Get CPU State
; 2rd dword on stack is output state
getstate:

    push [esp + 4]                      ; output state
    call getregs
    add esp, 4

    mov ebp, [esp + 4]                 ; output_state
    test ebp, ebp                      ; output_state == NULL
    jz getstate_done
    
    xor eax, eax    
    mov ax, es
    mov [ebp + REGS.es], eax

    mov ax, cs
    mov [ebp + REGS.cs], eax

    mov ax, ss
    mov [ebp + REGS.ss], eax

    mov ax, ds
    mov [ebp + REGS.ds], eax

    mov ax, fs
    mov [ebp + REGS.fs], eax

    mov ax, gs
    mov [ebp + REGS.gs], eax

    pushfd
    pop eax
    mov [ebp + REGS.eflags], eax
getstate_done:
    ret

; int86;
; 2nd dword on stack is int vector
; 3rd dword on stack is input registers pointer
; 4th dword on stack is output state pointer
int86:
    ;pusha                              ; save register state

    mov al, [esp + 4]                   ; vector
    mov [vec], al    

    push [esp + 8]                      ; input regs
    call setregs
    add esp, 4

    ; INT ib
    db 0xCD
vec db 0x00
     
    push [esp + 12]                      ; output state
    call getstate
    add esp, 4

    ;popa                               ; restore regiser state
    ret
    
; inb;
; 2nd dword on stack is port
; returns value in AL
inb:
    mov edx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

; inw;
; 2nd dword on stack is port
; returns value in AX
inw:
    mov edx, [esp + 4]
    xor eax, eax
    in ax, dx
    ret
   
; ind;
; 2nd dword on stack is port
; returns value in EAX
ind:
    mov edx, [esp + 4]
    xor eax, eax
    in eax, dx
    ret
    
; outb;
; 2nd dword on stack is port
; 3rd dword on stack is value
outb:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, al
    ret

; outw;
; 2nd dword on stack is port
; 3rd dword on stack is value
outw:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, ax
    ret

; outd;
; 2nd dword on stack is port
; 3rd dword on stack is value
outd:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, eax
    ret
 
 
; Write interrupt gate
; offset
; selector
; ar byte
; vector
write_int_gate:
    push ebx
    push esi
    push edi

    xor ecx, ecx
    mov cl, [esp + 12+4]       ; vector
    mov esi, [esp + 12+8]      ; offset
    mov bl, [esp + 12+12]      ; ar byte
    mov ax, [esp + 12+16]      ; selector
    mov edi, [idt_base]        ; idt base

    mov [edi+ecx*8+0], si      ; offset lower 16bit
    mov [edi+ecx*8+2], ax      ; selector
    mov byte [edi+ecx*8+4], 0
    mov [edi+ecx*8+5], bl      ; ar
    shr esi, 16
    mov [edi+ecx*8+6], si      ; offset upper 16bit

    pop edi
    pop esi
    pop ebx
    ret
    
; Write call gate
; offset
; selector
; ar byte
; index
write_call_gate:
    ret
      
; Write tss gate
; selector
; limit
; ar
; base
write_tss_gate:
    push ebx
    push esi
    push edi

    mov ecx, [esp + 12+4]      ; selector
    mov esi, [esp + 12+8]      ; limit
    mov ebx, [esp + 12+12]     ; ar byte
    mov eax, [esp + 12+16]     ; base
    mov edi, [gdt_base]        ; gdt base
    mov edx, esi
    
    and ecx, 0xFFF8            ; selector & 0xFFF8
    
    mov [edi+ecx*1+0], si      ; limit lower 16bit
    mov [edi+ecx*1+2], ax      ; base lower 16bit
    shr eax, 16
    mov byte [edi+ecx*1+4], al ; base upper 8bit
    mov [edi+ecx*1+5], bl      ; ar byte
    shr esi, 16
    and si, 0xF
    shr bx, 8
    and bl, 0xF0
    or bx, si
    mov [edi+ecx*1+6], bl      ; ar; limit 19:16
    mov byte [edi+ecx*1+7], ah ; base upper 8bit

    pop edi
    pop esi
    pop ebx
    ret

; Write task gate
; offset
; selector
; ar
; index
write_task_gate:
    ret

setcr0:
    mov eax, [esp+4]
    mov cr0, eax
    ret

getcr0:
    mov eax, cr0
    ret
    
setcr2:
    mov eax, [esp+4]
    mov cr2, eax
    ret
    
getcr2:
    mov eax, cr2
    ret
    
setcr3:
    mov eax, [esp+4]
    mov cr3, eax
    ret
    
getcr3:
    mov eax, cr3
    ret