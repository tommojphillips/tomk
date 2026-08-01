
IDT_BASE    equ 0x00C00000 ; IDT base address
GDT_BASE    equ 0x00B00000 ; GDT base address

PD_BASE     equ 0x1000     ; Page directory base address
PT_BASE     equ 0x2000     ; Page table base address
PAGE_SIZE   equ 0x1000     ; Page table size

KCODE       equ 0x08       ; Kernel code segment
KDATA       equ 0x10       ; Kernel data segment
KSTACK      equ 0x18       ; Kernel stack segment

MEMORY_TEST equ 0
