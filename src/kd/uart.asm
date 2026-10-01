; uart.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; uart
;

global serial_init
global serial_write
global serial_read

COM1        equ 0x3F8
COM2        equ 0x2F8
COM3        equ 0x3E8
COM4        equ 0x2E8
COM5        equ 0x5F8
COM6        equ 0x4F8
COM7        equ 0x5E8
COM8        equ 0x4E8

BAUD_38400  equ 0x03
BAUD        equ BAUD_38400

DLAB        equ 0x80                             ; (LCR bit) divisor latch enable
_8N1        equ 0x03                             ; (LCR bit) 

DR          equ 0x01                             ; (LSR bit) data ready
THRE        equ 0x20                             ; (LSR bit) transmitter holding register empty
TEMT        equ 0x40                             ; (LSR bit) transmitter shift register empty

TEST_BYTE   equ 0xAE

DATA         equ 0                               ; Data register (+0)
IER          equ 1                               ; Interrupt Enable Register (+1)
FCR          equ 2                               ; FIFO Control Registers (+2)
LCR          equ 3                               ; Line Control Register (+3)
MCR          equ 4                               ; Modem Control Register (+4)
LSR          equ 5                               ; Line Status Register (+5)
MSR          equ 6                               ; Modem Status Register (+6)

DLL          equ 0                               ; Divisor latch least significant byte
DLM          equ 1                               ; Divisor latch most significant byte

%macro READ 1
    mov dx, bx
    add dx, %1
    in al, dx
%endmacro

Section .text

; Serial init
; esp+4 = COM port
serial_init:
    push ebx
    push edx

    mov bx, [esp+8+4]                            ; COM port

    ; disable interrupts
    mov dx, bx
    add dx, IER
    mov al, 0
    out dx, al

    ; enable DLAB
    mov dx, bx
    add dx, LCR
    mov al, DLAB
    out dx, al
    
    ; set divisor
    mov dx, bx
    add dx, DLL
    mov al, BAUD & 0xFF
    out dx, al

    mov dx, bx
    add dx, DLM
    mov al, BAUD >> 8
    out dx, al

    ; disable DLAB; set 8N1
    mov dx, bx
    add dx, LCR
    mov al, _8N1
    out dx, al

    ; enable FIFO, clear them, 14-byte threshold
    mov dx, bx
    add dx, FCR
    mov al, 0xC7
    out dx, al

    ; set DTR, RTS, OUT2
    mov dx, bx
    add dx, MCR
    mov al, 0x0B
    out dx, al

.self_test:
    ; set loopback mode
    mov dx, bx
    add dx, MCR
    mov al, 0x1E
    out dx, al

    ; send test byte
    mov dx, bx
    add dx, DATA
    mov al, TEST_BYTE
    out dx, al

    ; wait for data
    mov cx, 0xFFFF                               ; tries
.lp:
    ; read LSR
    mov dx, bx
    add dx, LSR
    in al, dx

    test al, DR                                  ; data ready?
    jnz .recieve_data                            ; yes, read data
    dec cx                                       ; count -= 1
    jnz .lp                                      ; count == 0?
    jmp .done                                    ; yes, timeout error

.recieve_data:
    ; recieve test data
    mov dx, bx
    add dx, DATA
    in al, dx

    ; check if correct byte
    cmp al, TEST_BYTE
    jnz .done

    ; set to normal operation mode
    mov dx, bx
    add dx, MCR
    mov al, 0x0F
    out dx, al

.done:
    pop edx
    pop ebx
    ret

; Serial read byte
; esp+4 = COM port
serial_read:
    push ebx
    push edx

    mov bx, [esp+8+4]                            ; COM port

    ; read LSR
    mov dx, bx
    add dx, LSR
    in al, dx

    ; check for data
    test al, DR                                  ; data ready?
    mov eax, 0
    jz .done                                     ; no, done

    ; recieve data
    mov dx, bx
    add dx, DATA
    in al, dx                                    ; read data

.done:
    pop edx
    pop ebx
    ret

; Serial write byte
; esp+4 = COM port
; esp+8 = byte
serial_write:    
    push ebx
    push edx

    mov bx, [esp+8+4]                            ; COM port
    mov cx, 0xFFFF                               ; tries
.lp:
    ; read LSR
    mov dx, bx
    add dx, LSR
    in al, dx

    ; wait
    test al, THRE                                ; can accept another byte?
    jnz .write_data                              ; yes, write data
    dec cx                                       ; count -= 1
    jnz .lp                                      ; count == 0?
    jmp .done                                    ; yes, timeout error

.write_data:
    ; write data
    mov dx, bx
    add dx, DATA
    mov al, [esp+8+8]
    out dx, al

.done:
    pop edx
    pop ebx
    ret
