; pit.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;

BITS 32

extern write_int_gate
extern pic_enable_irq
extern pic_send_eoi
extern PIC_BASE

global pit_init
global timer_ticks
global wait_ms

IRQ0        equ 0

PIT_FREQ    equ 1193182
PIT_HZ      equ 1000
PIT_DIVISOR equ (PIT_FREQ/PIT_HZ)

PIT_PORT_1  equ 0x40
PIT_PORT_2  equ 0x43

%include "src\kernel\include\common.inc"

Section .bss
    timer_ticks dd ?

Section .text

pit_init:

    ; IRQ 0
    push KCODE                         ; selector
    push 10001110b                     ; access P=1 DPL=00 S=0 TYPE=1110 (INT 386)    
    push timer_int_handler             ; offset
    push PIC_BASE+IRQ0                 ; vector 
    call write_int_gate
    add esp, 16

    ; b00110100
    ; b00xxxxxx = Channel 0
    ; bXX11xxxx = lo/hi byte
    ; bxxxx010x = mode 2
    ; bxxxxxxx0 = binary
    mov al, 0x34
    out PIT_PORT_2, al

    mov ax, PIT_DIVISOR
    out PIT_PORT_1, al
    
    shr ax, 8
    out PIT_PORT_1, al    

    push IRQ0                          ; IRQ0 (TIMER)
    call pic_enable_irq
    add esp, 4

    mov dword [timer_ticks], 0

    ret

timer_int_handler:
    pusha

    push IRQ0
    call pic_send_eoi
    add esp, 4

    inc dword [timer_ticks]
    
.done:
    popa
    iret
