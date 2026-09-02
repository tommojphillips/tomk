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

%include "src\kernel\include\common.inc"

IRQ1        equ 1
KB_BUF_SIZE equ 32

Section .rodata
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
    kb_buf resb KB_BUF_SIZE
    kb_idx dd ?

Section .text

ps2_init:
    mov dword [kb_idx], 0             ; kb_idx = 0;

    ; IRQ 1
    push ps2_int_handler               ; offset
    push KCODE                         ; selector    
    push PIC_BASE+IRQ1                 ; vector 
    call write_int_gate
    add esp, 12

    push IRQ1                          ; IRQ1 (KB)
    call pic_enable_irq
    add esp, 4

    ret

ps2_int_handler:
    pusha

    push 1
    call pic_send_eoi
    add esp, 4
    
    mov ebx, [kb_idx]
    cmp ebx, KB_BUF_SIZE
    jge .done

    in al, 0x60

    cmp al, 0xE0
    jz .done

    test al, 0x80
    jnz .done

    mov [kb_buf+ebx], al

    inc ebx
    mov [kb_idx], ebx

.done:
    popa
    iret

; Reset CPU
ps2_cpu_reset:
    cli
    mov  al, 0xFE       ; yes, reset cpu.
    out  0x64, al

    .lp:
    jmp .lp             ; spin

; Convert SCANCODE to CHARACTER
; scancode
; returns 0 if invalid. otherwise character
ps2_sc2ch:
    mov eax, [esp+4]
    movzx eax, byte [sc_ch+eax]
    ret

; Get SCANCODE
; returns 0 if invalid. otherwise scancode
ps2_getscancode:
    push ebx

    mov eax, [kb_idx]
    
    mov ebx, eax
    xor eax, eax

    test ebx, ebx
    jz .done

    dec ebx
    mov [kb_idx], ebx

    movzx eax, byte [kb_buf+ebx]

.done:
    pop ebx
    ret

; Get CHARACTER
; returns 0 if invalid. otherwise character
ps2_getchar:
    call ps2_getscancode

    test eax, eax
    jz .done

    push eax              ; convert scancode to character
    call ps2_sc2ch
    add esp, 4

.done:
    ret
