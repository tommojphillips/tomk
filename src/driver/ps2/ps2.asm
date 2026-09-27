; ps2.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; PS/2 keyboard driver
;

BITS 32

extern write_int_gate
extern printf
extern pic_enable_irq
extern pic_send_eoi
extern PIC_BASE

global ps2_init
global ps2_poll
global ps2_getchar
global ps2_getscancode
global ps2_sc2ch
global ps2_cpu_reset

%include "src\include\common.inc"

IRQ1        equ 1  ; PIC keyboard irq number 
KB_BUF_SIZE equ 32 ; Assumed to be power of 2

Section .rodata
    ; scancode to ascii lookup table
    sc_ch  db \
    0, 0x1B, '1', '2', \
    '3', '4', '5', '6', \
    '7', '8', '9', '0', \
    '-', '=', 0x08, 0, \
    'Q', 'W', 'E', 'R', \
    'T', 'Y', 'U', 'I', \
    'O', 'P', '[', ']', \
    0x0D, 0, 'A', 'S', \
    'D', 'F', 'G', 'H', \
    'J', 'K', 'L', ';', \
    0, '`', 0, 0, \
    'Z', 'X', 'C', 'V', \
    'B', 'N', 'M', ',', \
    '.', '/', 0, '*', \
    0, ' ', 0, 0, \
    0, 0, 0, 0, \
    0, 0, 0, 0, \
    0, 0, 0, 0, \
    0, 0, '-', 0, \
    0, 0, '+', 0, \
    0, 0, 0, 0, \
    0, 0, 0, 0, \
    0, 0, 0, 0, \

Section .bss
    buff  resb KB_BUF_SIZE
    head  dd ?
    tail  dd ?
    count dd ?

Section .text

ps2_init:
    mov dword [head], 0                ; rb->head = 0
    mov dword [tail], 0                ; rb->tail = 0
    mov dword [count], 0               ; rb->count = 0

    ; IRQ 1
    push ps2_int_handler               ; offset
    push gdt_selector_ke_code          ; selector    
    push PIC_BASE+IRQ1                 ; vector 
    call write_int_gate
    add esp, 12

    push IRQ1                          ; IRQ1 (KB)
    call pic_enable_irq
    add esp, 4

    ret

; ps/2 int handler
; esp+0  = eip
; esp+4  = cs
; esp+8  = eflags
; esp+12 = esp2 ( if priv change )
; esp+16 = ss2 ( if priv change )
ps2_int_handler:
    pusha

    ; Send EOI to PIC
    push IRQ1
    call pic_send_eoi
    add esp, 4

    ; Get byte from ps/2
    in al, 0x60

    ; Extended opcode prefix?
    cmp al, 0xE0
    jz .done ; yes, ignore it

    ; Key release ?
    test al, 0x80
    jnz .done ; yes, ignore it

    ; rb->buffer[rb->tail] = item
    mov ebx, [tail]
    mov [buff+ebx], al
    
    ; rb->tail = (rb->tail + 1) % rb->buffer_size
    inc dword [tail]
    and dword [tail], KB_BUF_SIZE-1

    ; if (rb->count < rb->buffer_size)
    cmp dword [count], KB_BUF_SIZE
    jge .overrun

    ; rb->count++
    inc dword [count]
    jmp .done

.overrun:
    ; Head has been overrun; inc head so pop reads oldest val first
    ; rb->head = (rb->head + 1) % rb->buffer_size
    inc dword [head]
    and dword [head], KB_BUF_SIZE-1

.done:
    popa
    iret

; Reset CPU
ps2_cpu_reset:
    cli
    mov  al, 0xFE ; yes, reset cpu.
    out  0x64, al

.lp:
    jmp .lp ; spin

; Convert ps/2 scancode to ascii character
; esp+4 = ps/2 scancode
; returns 0 if invalid. otherwise character
ps2_sc2ch:
    mov eax, [esp+4]
    movzx eax, byte [sc_ch+eax]
    ret

; Get ps/2 scancode
; returns 0 if invalid. otherwise scancode
ps2_getscancode:
    push ebx
    
    ; ret val
    xor eax, eax

    ; if (rb->count == 0)
    mov ecx, [count]
    test ecx, ecx
    jz .done

    ; item = rb->buffer[rb->head]
    mov ebx, [head]

    ; rb->head = (rb->head + 1) % rb->buffer_size
    inc dword [head]
    and dword [head], KB_BUF_SIZE-1

    ; rb->count--
    dec dword [count]

    movzx eax, byte [buff+ebx] ; item

.done:
    pop ebx
    ret

; Get ascii character
; returns 0 if invalid. otherwise character
ps2_getchar:
    call ps2_getscancode

    ; Scancoode invalid ?
    test eax, eax
    jz .done ; yes, done

    ; Convert ps/2 scancode to ascii character
    push eax
    call ps2_sc2ch
    add esp, 4

.done:
    ret
