; uart.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; uart
;

global serial_init
global serial_write
global serial_read

PORT equ 0x3F8

section .text

serial_init:
    push edx
    mov al, 0x00
    mov dx, PORT+1
    out dx, al                                   ; disable all interrupts    
    
    mov al, 0x80
    mov dx, PORT+3
    out dx, al                                   ; enable DLAB (set baud rate divisor)
    
    mov al, 0x03
    mov dx, PORT+0
    out dx, al                                   ; set divisor to 3 (lo byte) 38400 baud
    
    mov al, 0x00
    mov dx, PORT+1
    out dx, al                                   ; (hi byte)
    
    mov al, 0x03
    mov dx, PORT+3
    out dx, al                                   ; 8 bits, no parity, one stop bit
    
    mov al, 0xC7
    mov dx, PORT+2
    out dx, al                                   ; enable FIFO, clear them, with 14-byte threshold
    
    mov al, 0x0B
    mov dx, PORT+4
    out dx, al                                   ; IRQs enabled, RTS/DSR set
    
    mov al, 0x1E
    mov dx, PORT+4
    out dx, al                                   ; set in loopback mode
    
    mov al, 0xAE
    mov dx, PORT+0
    out dx, al                                   ; check if serial is faulty
    in al, dx
    cmp al, 0xAE                                 ; serial chip should return 0xAE in PORT+0
    jnz .err

    mov al, 0x0F
    mov dx, PORT+4
    out dx, al                                   ; set to normal operation mode
    
    xor eax, eax
    jmp .done

.err:
    mov eax, 1

.done:
    pop edx
    ret

serial_read:
    push edx

    xor eax, eax

    mov dx, PORT+5    
    in al, dx

    test al, 0x01                                ; byte received?
    jz .done                                     ; no, done
    
    mov dx, PORT+0
    in al, dx                                    ; read byte

.done:
    pop edx
    ret

serial_write:
    push edx
    
    mov dx, PORT+5
    in al, dx

    test al, 0x20                                ; transmit empty?
    jz .done                                     ; no, done
    
    mov al, [esp+4+4]
    mov dx, PORT+0
    out dx, al                                   ; write byte

.done:
    pop edx
    ret
