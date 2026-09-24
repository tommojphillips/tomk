; pit.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;

BITS 32

; Debugging
%define PIT_DBG 1

extern write_int_gate
extern pic_enable_irq
extern pic_disable_irq
extern pic_send_eoi
extern PIC_BASE

%ifdef PIT_DBG
extern kdprintf
%endif

global pit_init
global pit_set_handler
global pit_enable
global pit_disable
global pit_set_freq

IRQ0        equ 0
PIT_CLOCK   equ 1193182
PIT_PORT_1  equ 0x40
PIT_PORT_2  equ 0x43

%include "src\include\common.inc"

%ifdef PIT_DBG
section .rodata
    dbg_set_interval_str db "[PIT] set_freq clock=%u target=%uHz divisor=%u", 10, 0
    dbg_enable_str       db "[PIT] enable_timer", 10, 0
    dbg_set_handler_str  db "[PIT] set_handler handler=0x%08X", 10, 0
%endif

section .text

pit_init:

    call pit_disable

    ; b00110100
    ; b00xxxxxx = Channel 0
    ; bXX11xxxx = lo/hi byte
    ; bxxxx010x = mode 2
    ; bxxxxxxx0 = binary
    mov al, 0x34
    out PIT_PORT_2, al

    ret

; set PIT timer int handler
; esp+4 = int handler
pit_set_handler:

    mov eax, [esp+4]
    
%ifdef PIT_DBG
    ; Debug
    push eax                                     ; save handler
    push eax                                     ; handler
    push dbg_set_handler_str
    call kdprintf
    add esp, 8
    pop eax                                      ; restore handler
%endif

    ; Write interrupt gate IDT
    push eax                                     ; offset
    push KCODE                                   ; selector
    push PIC_BASE+IRQ0                           ; vector 
    call write_int_gate
    add esp, 12
    ret

; enable pit
pit_enable:

%ifdef PIT_DBG
    ; Debug
    push dbg_enable_str
    call kdprintf
    add esp, 4
%endif

    push IRQ0                                    ; IRQ0 (TIMER)
    call pic_enable_irq
    add esp, 4
    ret

; disable pit
pit_disable:
    push IRQ0                                    ; IRQ0 (TIMER)
    call pic_disable_irq
    add esp, 4
    ret

; set PIT timer frequency
; esp+4 = hz
pit_set_freq:
    push ebx
    push edx

    mov ebx, [esp+8+4]                           ; hz
    
    test ebx, ebx
    jnz .notzero

.zero:
    ; divisor = 0
    xor ax, ax
    jmp .set_divisor

.notzero:
    ; divisor = (freq/hz)
    xor edx, edx
    mov eax, PIT_CLOCK
    div ebx

.set_divisor:

%ifdef PIT_DBG
    ; Debug
    push eax                                     ; save divisor    
    push eax                                     ; divisor
    push ebx                                     ; hz
    push PIT_CLOCK                               ; clock
    push dbg_set_interval_str
    call kdprintf
    add esp, 16
    pop eax                                      ; restore divisor
%endif

    ; write lo byte of divisor
    out PIT_PORT_1, al

    ; write hi byte of divisor
    shr ax, 8
    out PIT_PORT_1, al 

    pop edx
    pop ebx
    ret
