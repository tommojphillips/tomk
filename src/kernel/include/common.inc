
KBASE       equ 0x00500000                       ; kernel base address
KCODE       equ 0x0008                           ; Kernel code segment
KDATA       equ 0x0010                           ; Kernel data segment
KSTACK      equ 0x0018                           ; Kernel stack segment

IDT_BASE    equ KBASE                            ; IDT base address
IDT_SIZE    equ 0x00000400 

GDT_BASE    equ KBASE+IDT_SIZE                   ; GDT base address
GDT_SIZE    equ 0xC00

PD_BASE     equ KBASE+IDT_SIZE+GDT_SIZE          ; Page directory base address          006FF000 - 006FFFFF    
PD_SIZE     equ 0x1000

PT_BASE     equ KBASE+IDT_SIZE+GDT_SIZE+PD_SIZE  ; Page table base address              00700000 - 00AFFFFF
PT_SIZE     equ 0x00400000

PAGE_SIZE   equ 0x1000                           ; Page table size
PD_COUNT    equ 0x0400                           ; Page directory count

