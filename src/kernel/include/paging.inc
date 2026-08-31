
NP         equ 0x00            ; 0 = NP
P          equ 0x01            ; 0 = NP; 1 = Present
RO         equ 0x00            ; 0 = RO;
RW         equ 0x02            ; 0 = RO; 1 = RW
US         equ 0x04            ; 0 = Super; 1 = User

PAGE_SIZE  equ 0x1000          ; 4 KiB page size
PD_SIZE    equ PAGE_SIZE
PD_COUNT   equ 0x0400          ; Page directory count
PT_SIZE    equ PAGE_SIZE*PD_COUNT

BIT_PG equ 0x80000000
BIT_WP equ 0x00010000
