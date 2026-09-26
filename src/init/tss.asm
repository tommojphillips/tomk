; tss.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;

BITS 32

extern write_tss_descriptor
extern kstack_top

global tss_init

; TSS struct
struc TSS
    .back_link    resw 1
    .res0         resw 1

    .esp0         resd 1
    .ss0          resw 1
    .res1         resw 1

    .esp1         resd 1
    .ss1          resw 1
    .res2         resw 1

    .esp2         resd 1
    .ss2          resw 1
    .res3         resw 1

    .cr3          resd 1
    .eip          resd 1
    .eflags       resd 1
    .eax          resd 1
    .ecx          resd 1
    .edx          resd 1
    .ebx          resd 1
    .esp          resd 1
    .ebp          resd 1
    .esi          resd 1
    .edi          resd 1

    .es           resw 1
    .res4         resw 1
    .cs           resw 1
    .res5         resw 1
    .ss           resw 1
    .res6         resw 1
    .ds           resw 1
    .res7         resw 1
    .fs           resw 1
    .res8         resw 1
    .gs           resw 1
    .res9         resw 1    
    .ldt          resw 1
    .res10        resw 1

    .t            resw 1
    .iomapbase    resw 1
endstruc

TSS_SIZE equ 0x68

%include "src\include\common.inc"

Section .bss
    align 8
    ke_tss resb TSS_SIZE

Section .text

tss_init:

    ;
    ; Kernel TSS
    ;

    ; Write TSS
    push ke_tss                                  ; base
    push 10001001b                               ; access P=1 DPL=00 S=0 TYPE=1001 (386 Available TSS)
    push TSS_SIZE-1                              ; limit
    push gdt_selector_ke_tss                     ; selector 
    call write_tss_descriptor
    add esp, 16

    ; Load TR
    mov ax, gdt_selector_ke_tss                  ; tss0
    ltr ax

    ; Setup kernel stack
    mov dword [ke_tss+TSS.esp0], kstack_top
    mov word [ke_tss+TSS.ss0], gdt_selector_ke_stack

    ret
