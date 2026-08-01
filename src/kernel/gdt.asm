BITS 32

global gdt_init

%include "src\kernel\include\common.inc"

section .rodata

align 8, db 0
gdt_start:

gdt_null:                          ; NULL
    dq 0

gdt_kernel_code:                   ; CODE 0x00000000-0xFFFFFFFF
    dw 0xFFFF                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x00                        ; base 23:16
    db 10011010b                   ; P=1 DPL=00 S=1 E=1 R=1
    db 11001111b                   ; gran=4k, 32-bit, limit 19:16
    db 0x00                        ; base 31:24

gdt_kernel_data:                   ; DATA 0x00000000-0xFFFFFFFF
    dw 0xFFFF                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x00                        ; base 23:16
    db 10010010b                   ; P=1 DPL=00 S=1 E=0 W=1
    db 10001111b                   ; gran=4k, limit 19:16
    db 0x00                        ; base 31:24

gdt_kernel_stack:                  ; STACK 0x00900000-0x009FFFFF
    dw 0xFFFF                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x00                        ; base 23:16
    db 10010010b                   ; P=1 DPL=00 S=1 E=0 W=1
    db 11001111b                   ; gran=4k, big=1, limit 19:16
    db 0x00                        ; base 31:24

gdt_user_code:                     ; CODE 0x00000000-0xFFFFFFFF
    dw 0xFFFF                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x00                        ; base 23:16
    db 11111010b                   ; P=1 DPL=11 S=1 E=1 R=1
    db 11001111b                   ; gran=4k, 32-bit, limit 19:16
    db 0x00                        ; base 31:24

gdt_user_data:                     ; DATA 0x00000000-0xFFFFFFFF
    dw 0xFFFF                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x00                        ; base 23:16
    db 11110010b                   ; P=1 DPL=11 S=1 E=0 W=1
    db 10001111b                   ; gran=4k, limit 19:16
    db 0x00                        ; base 31:24

gdt_user_stack:                    ; STACK 0x00900000-0x009FFFFF
    dw 0xFFFF                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x00                        ; base 23:16
    db 11110010b                   ; P=1 DPL=11 S=1 E=0 W=1
    db 11001111b                   ; gran=4k, big=1, limit 19:16
    db 0x00                        ; base 31:24

gdt_tss0:                          ; TSS0 0x00600000-0x00600068
    dw 0x0068                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x60                        ; base 23:16
    db 10001001b                   ; P=1 DPL=00 S=0 Type=1001 (386 Available TSS)
    db 0x00                        ; limit 19:16
    db 0x00                        ; base 31:24
  
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; gdt limit - 1
    dd gdt_start               ; gdt base

section .text

gdt_init:
    ; create copy of gdt in ram
    lea eax, [gdt_descriptor]
    xor ecx, ecx
    mov cx, [eax+0]              ; gdt len-1
    mov esi, [eax+2]             ; old gdt base
    mov edi, GDT_BASE            ; new gdt base
    rep movsb

    ; write gdt descriptor
    xor ecx, ecx
    mov cx, [eax+0]              ; gdt len-1
    mov edi, GDT_BASE            ; new gdt base
    add edi, ecx                 ; base+len
    inc edi
    mov [edi+0], cx
    mov dword [edi+2], GDT_BASE
    
    lgdt [edi]
    
    ; Reload data sregs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Reload stack sreg
    mov ebx, [esp]
    mov ax, 0x18
    mov ss, ax
    mov esp, 0x00910000

    jmp ebx ; ret
