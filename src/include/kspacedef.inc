; kspacedef.inc
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;

KVIRT equ 0xC0000000                         ; Kernel virtual address base

%define V2P(_virt_addr) (_virt_addr - KVIRT) ; Converts kernel virtual address to physical address
%define P2V(_virt_addr) (KVIRT + _virt_addr) ; Converts physical address to kernel virtual address
