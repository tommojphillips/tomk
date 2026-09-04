; uart.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; uart
;

global serial_init
global serial_write
global serial_read

PORT        equ 0x3F8

BAUD_38400  equ 0x03
BAUD        equ BAUD_38400

LCR_DLAB    equ 0x80
LCR_TEMT    equ 0x40                              ; transmitter shift register empty bit
LCR_THRE    equ 0x20                              ; transmitter holding register empty bit
LCR_8N1     equ 0x03
LSR_DR      equ 0x01                              ; data ready bit
TEST_BYTE   equ 0xAE

section .text

serial_init:
    push edx

    mov al, 0x00
    mov dx, PORT+1
    out dx, al                                   ; IER = 0; disable interrupts    

.set_baud:
    mov al, LCR_DLAB
    mov dx, PORT+3                               ; LCR
    out dx, al                                   ; enable DLAB in LCR
    
    mov ax, BAUD
    mov dx, PORT+0                               ; DLL
    out dx, al                                   ; set divisor to 3 38400 baud
    
    shr ax, 8
    mov dx, PORT+1                               ; DLM
    out dx, al                                   ; set divisor to 3 38400 baud

    mov al, LCR_8N1
    mov dx, PORT+3
    out dx, al                                   ; set 8 bits, no parity, one stop bit; disable DLAB
    
    mov al, 0xC7
    mov dx, PORT+2                               ; FCR
    out dx, al                                   ; enable FIFO, clear them, with 14-byte threshold
    
    mov al, 0x0B
    mov dx, PORT+4                               ; MCR
    out dx, al                                   ; IRQs enabled, RTS/DSR set

.self_test:
    mov al, 0x1E
    mov dx, PORT+4                               ; MCR
    out dx, al                                   ; set in loopback mode
    
    mov al, TEST_BYTE
    mov dx, PORT+0
    out dx, al                                   ; THR - check if serial is faulty
    
    mov dx, PORT+5                               ; LSR
    in al, dx
    test al, LSR_DR                              ; data ready?
    jz .done

    mov dx, PORT+0
    in al, dx                                    ; RBR
    cmp al, TEST_BYTE                            ; serial chip should return 0xAE in PORT+0
    jnz .done

    mov al, 0x0F
    mov dx, PORT+4                               ; MCR
    out dx, al                                   ; set to normal operation mode

.done:
    pop edx
    ret

serial_read:
    push edx

    mov dx, PORT+5                               ; LSR
    in al, dx

    test al, LSR_DR                              ; data ready?
    jz .err                                      ; no, done
    
    xor eax, eax
    mov dx, PORT+0
    in al, dx                                    ; read byte
    jmp .done

.err:
    pop edx
    xor eax, eax
    ret

.done:
    pop edx
    ret

serial_write:
    push edx
    
    mov dx, PORT+5                               ; LSR
    in al, dx

    test al, LCR_THRE                            ; can accept another byte?
    jz .done                                     ; no, done
    
    mov al, [esp+4+4]
    mov dx, PORT+0
    out dx, al                                   ; write byte

.done:
    pop edx
    ret
