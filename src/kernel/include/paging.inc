
P          equ 0x01            ; 0 = NP; 1 = Present
RW         equ 0x02            ; 0 = RO; 1 = RW
US         equ 0x04            ; 0 = Super; 1 = User
A          equ 0x20            ; 0 = not accessed; 1 = accessed
D          equ 0x40            ; 0 = not dirty; 1 = dirty

PAGE_SIZE  equ 0x1000          ; 4 KiB page size
PD_SIZE    equ PAGE_SIZE
PD_COUNT   equ 0x0400          ; Page directory count
PT_SIZE    equ PAGE_SIZE*PD_COUNT