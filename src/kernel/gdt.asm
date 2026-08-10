; gdt.asm 

BITS 32

global gdt_init
global KCODE
global KDATA
global KSTACK
global KTSS
global UCODE
global UDATA
global USTACK
global gdt

KCODE   equ gdt_kcode  - gdt       ; kernel code segment
KDATA   equ gdt_kdata  - gdt       ; kernel data segment
KSTACK  equ gdt_kstack - gdt       ; kernel stack segment
KTSS    equ gdt_tss    - gdt       ; Kernel tss segment

UCODE   equ (gdt_ucode  - gdt) | 3 ; user code segment
UDATA   equ (gdt_udata  - gdt) | 3 ; user data segment
USTACK  equ (gdt_ustack - gdt) | 3 ; user stack segment

Section .data

align 8, db 0
gdt:
gdt_start:

gdt_null:                          ; NULL
    dq 0

gdt_kcode:                         ; CODE 0x00000000-0xFFFFFFFF
    dw 0xFFFF                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x00                        ; base 23:16
    db 10011010b                   ; P=1 DPL=00 S=1 E=1 R=1
    db 11001111b                   ; gran=4k, 32-bit, limit 19:16
    db 0x00                        ; base 31:24

gdt_kdata:                         ; DATA 0x00000000-0xFFFFFFFF
    dw 0xFFFF                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x00                        ; base 23:16
    db 10010010b                   ; P=1 DPL=00 S=1 E=0 W=1
    db 10001111b                   ; gran=4k, limit 19:16
    db 0x00                        ; base 31:24

gdt_kstack:                        ; STACK 0x00900000-0x009FFFFF
    dw 0xFFFF                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x00                        ; base 23:16
    db 10010010b                   ; P=1 DPL=00 S=1 E=0 W=1
    db 11001111b                   ; gran=4k, big=1, limit 19:16
    db 0x00                        ; base 31:24

gdt_ucode:                         ; CODE 0x00000000-0xFFFFFFFF
    dw 0xFFFF                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x00                        ; base 23:16
    db 11111010b                   ; P=1 DPL=11 S=1 E=1 R=1
    db 11001111b                   ; gran=4k, 32-bit, limit 19:16
    db 0x00                        ; base 31:24

gdt_udata:                         ; DATA 0x00000000-0xFFFFFFFF
    dw 0xFFFF                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x00                        ; base 23:16
    db 11110010b                   ; P=1 DPL=11 S=1 E=0 W=1
    db 10001111b                   ; gran=4k, limit 19:16
    db 0x00                        ; base 31:24

gdt_ustack:                        ; STACK 0x00900000-0x009FFFFF
    dw 0xFFFF                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x00                        ; base 23:16
    db 11110010b                   ; P=1 DPL=11 S=1 E=0 W=1
    db 11001111b                   ; gran=4k, big=1, limit 19:16
    db 0x00                        ; base 31:24

gdt_tss:                           ; TSS - dummy; patched in tss.asm
    dw 0x0000                      ; limit 15:0
    dw 0x0000                      ; base 15:0
    db 0x00                        ; base 23:16
    db 00000000b                   ; P=1 DPL=00 S=0 Type=1001 (386 Available TSS)
    db 00000000b                   ; gran=0, big=0, limit 19:16
    db 0x00                        ; base 31:24
  
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1     ; gdt limit - 1
    dd gdt_start                   ; gdt base

Section .text

gdt_init:
    
    lgdt [gdt_descriptor]
    
    ; Reload es/ds/fs/gs
    mov ax, KDATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Reload ss
    mov ax, KSTACK
    mov ss, ax

    ret
