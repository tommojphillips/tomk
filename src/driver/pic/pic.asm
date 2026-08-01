; pic.asm - Programmable interrupt controller
BITS 32

global pic_init
global pic_disable
global pic_send_eoi
global pic_enable_irq
global pic_disable_irq
global PIC_BASE

PIC_BASE equ 0x20         ; ICW2 - interrupt vector base

PIC1_CTRL equ 0x20
PIC2_CTRL equ 0xA0

PIC1_DATA equ 0x21
PIC2_DATA equ 0xA1

PIC1_IR_LN2 equ 0x4       ; ICW3 - 0100->IR Line 2
PIC2_IR_LN2 equ 0x2       ; ICW3 - 0010->IR line 2

ICW1 equ 0x11
ICW4 equ 0x01

NON_SPEC_EOI equ 0x20

Section .text

pic_init:
    call pic_reset
    call pic_disable
    ret

pic_reset:

    ; Send ICW1 to PIC1/PIC2
	mov	al, ICW1
	out	PIC1_CTRL, al
	out	PIC2_CTRL, al

    ; Send ICW2 to PIC1
	mov	al, PIC_BASE      ; PIC1 handles IRQ 0..7. map IRQ 0 to use interrupt PIC_BASE
	out	PIC1_DATA, al

	; Send ICW2 to PIC2
	mov	al, PIC_BASE+8    ; PIC2 handles IRQ's 8..15. map IRQ 8 to use interrupt PIC_BASE+8
	out	PIC2_DATA, al

    ; Send ICW3 to PIC1
	mov	al, PIC1_IR_LN2   ; 0100->IR Line 2
	out	PIC1_DATA, al

    ; Send ICW3 to PIC2
	mov	al, PIC2_IR_LN2   ; 0010->IR Line 2
	out	PIC2_DATA, al

	; Send ICW4
    mov	al, ICW4          ; bit 0 enables 80x86 mode 
	out	PIC1_DATA, al
    out	PIC2_DATA, al

    ; clear data registers
    mov al, 0
    out PIC1_DATA, al
    out PIC2_DATA, al

    ret

; PIC disable all IRQ
pic_disable:
    mov al, 0xFF
    out PIC1_DATA, al
    out PIC2_DATA, al    
    ret

; PIC send EOI
; irq: 8bit
pic_send_eoi:
    push ebx

    mov bl, [esp + 8]     ; irq
    mov al, NON_SPEC_EOI

    cmp bl, 8
    jb .master

.slave:
    out PIC2_CTRL, al
.master:
    out PIC1_CTRL, al

    pop ebx
    ret

; PIC enable IRQ
; irq: 8bit
pic_enable_irq:
    push ebx

    mov cl, [esp + 8]     ; irq

    cmp cl, 8
    jb .master

.slave:
    mov dx, PIC2_DATA
    sub cl, 8
    jmp .clr

.master:
    mov dx, PIC1_DATA

.clr:                     ; value = inb(port) & ~(1 << irq_line);
    in al, dx
    mov bl, 1
    shl bl, cl
    not bl
    and al, bl
    out dx, al

    pop ebx
    ret

; PIC disable IRQ
; irq: 8bit
pic_disable_irq:
    push ebx

    mov cl, [esp + 8]     ; irq

    cmp cl, 8
    jb .master

.slave:
    mov dx, PIC2_DATA
    sub cl, 8
    jmp .set

.master:
    mov dx, PIC1_DATA

.set:                     ; value = inb(port) | (1 << irq_line);
    in al, dx
    mov bl, 1
    shl bl, cl
    or al, bl
    out dx, al

    pop ebx
    ret
