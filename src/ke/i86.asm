; i86.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;

BITS 32

extern idt
extern gdt

global inb
global outb
global inw
global outw
global ind
global outd

global write_int_gate
global write_task_gate
global write_tss_descriptor

global getesp

global setcr0
global setcr2
global setcr3

global getcr0
global getcr2
global getcr3

global spinwait
global haltwait
global halt

Section .text

; inb;
; esp+4 = port
; returns value
inb:
    push edx
    mov dx, [esp+8]
    xor eax, eax
    in al, dx
    pop edx
    ret

; inw;
; esp+4 = port
; returns value
inw:
    push edx
    mov dx, [esp+8]
    xor eax, eax
    in ax, dx
    pop edx
    ret
   
; ind;
; esp+4 = port
; returns value
ind:
    push edx
    mov dx, [esp+8]
    xor eax, eax
    in eax, dx
    pop edx
    ret
    
; outb;
; esp+4 = port
; esp+8 = value
outb:
    push edx
    mov dx, [esp+8]
    mov al, [esp+12]
    out dx, al
    pop edx
    ret

; outw;
; esp+4 = port
; esp+8 = value
outw:
    push edx
    mov dx, [esp+8]
    mov ax, [esp+12]
    out dx, ax
    pop edx
    ret

; outd;
; esp+4 = port
; esp+8 = value
outd:
    push edx
    mov dx, [esp+8]
    mov eax, [esp+12]
    out dx, eax
    pop edx
    ret
  
; Write interrupt gate to IDT
; esp+4  = vector
; esp+8  = selector
; esp+12 = offset
write_int_gate:
    push esi
    push edi

    xor ecx, ecx
    mov cl, [esp+8+4]                            ; vector
    mov ax, [esp+8+8]                            ; selector
    mov esi, [esp+8+12]                          ; offset
    mov edi, idt                                 ; idt base
 
    mov [edi+ecx*8+0], si                        ; offset lower 16bit
    mov [edi+ecx*8+2], ax                        ; selector
    mov byte [edi+ecx*8+4], 0
    mov [edi+ecx*8+5], 10001110b                 ; P=1 DPL=00 S=0 TYPE=1110 (INT 386)  
    shr esi, 16
    mov [edi+ecx*8+6], si                        ; offset upper 16bit

    pop edi
    pop esi
    ret

; Write task gate to IDT
; esp+4 = vector
; esp+8 = TSS selector
write_task_gate:
    push ecx
    push edi

    xor ecx, ecx
    mov cl, [esp+8+4]                            ; vector
    mov ax,  [esp+8+8]                           ; TSS selector
    mov edi, idt                                 ; idt base

    mov [edi+ecx*8+0], ax                        ; TSS selector
    mov word [edi+ecx*8+2], 0
    mov word [edi+ecx*8+4], 0
    mov byte [edi+ecx*8+5], 10000101b            ; P=1 DPL=00 S=0 TYPE=0101 (TASK)
    mov word [edi+ecx*8+6], 0

    pop edi
    pop ecx
    ret

; Write tss descriptor to GDT
; esp+4  = selector
; esp+8  = limit
; esp+12 = ar
; esp+16 = base
write_tss_descriptor:
    push ebx
    push esi
    push edi

    mov ecx, [esp+12+4]        ; selector
    mov esi, [esp+12+8]        ; limit
    mov bx, [esp+12+12]        ; ar word
    mov eax, [esp+12+16]       ; base
    mov edi, gdt               ; gdt base
        
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

; Get ESP
; returns value
getesp:
    mov eax, esp
    ret

; Set CR0
; esp+4 = value
setcr0:
    mov eax, [esp+4]
    mov cr0, eax
    ret

; Get CR0
; returns value
getcr0:
    mov eax, cr0
    ret
    
; Set CR2
; esp+4 = value
setcr2:
    mov eax, [esp+4]
    mov cr2, eax
    ret
    
; Get CR2
; returns value
getcr2:
    mov eax, cr2
    ret
    
; Set CR3
; esp+4 = value
setcr3:
    mov eax, [esp+4]
    mov cr3, eax
    ret
    
; Get CR3
; returns value
getcr3:
    mov eax, cr3
    ret

; Set DR0
; esp+4 = value
setdr0:
    mov eax, [esp+4]
    mov dr0, eax
    ret
    
; Get DR0
; returns value
getdr0:
    mov eax, dr0
    ret

; Set DR1
; esp+4 = value
setdr1:
    mov eax, [esp+4]
    mov dr1, eax
    ret
    
; Get DR1
; returns value
getdr1:
    mov eax, dr1
    ret

; Set DR2
; esp+4 = value
setdr2:
    mov eax, [esp+4]
    mov dr2, eax
    ret
    
; Get DR2
; returns value
getdr2:
    mov eax, dr2
    ret

; Set DR3
; esp+4 = value
setdr3:
    mov eax, [esp+4]
    mov dr3, eax
    ret
    
; Get DR3
; returns value
getdr3:
    mov eax, dr3
    ret

; Set DR6
; esp+4 = value
setdr6:
    mov eax, [esp+4]
    mov dr6, eax
    ret
    
; Get DR6
; returns value
getdr6:
    mov eax, dr6
    ret

; Set DR7
; esp+4 = value
setdr7:
    mov eax, [esp+4]
    mov dr7, eax
    ret
    
; Get DR7
; returns value
getdr7:
    mov eax, dr7
    ret

; Enable interrupts
enable_interrupts:
    sti
    ret

; Disable interrupts
disable_interrupts:
    cli
    ret

; spin-wait x times
; esp+4 = amount
spinwait:
    mov eax, [esp+4]
    
    test eax, eax
    jz .done

.spin:
    dec eax
    jnz .spin

.done:
    ret

; halt cpu
haltwait:
    hlt
    ret

; halt cpu
halt:
    hlt
    jmp halt
