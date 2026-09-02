; common.inc
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;

extern KCODE                            ; Kernelspace code segment
extern KDATA                            ; Kernelspace data segment
extern KSTACK                           ; Kernelspace stack segment
extern KTSS                             ; Kernelspace tss

extern UCODE                            ; Userspace code segment
extern UDATA                            ; Userspace data segment
extern USTACK                           ; Userspace stack segment

KVIRT equ 0xC0000000                    ; higher-half kernel virtual address base

%define V2P(x) (x - KVIRT)              ; convert a higher-half kernel virtual address to a physical address
%define P2V(x) (KVIRT + x)              ; convert a physical address to a higher-half kernel virtual address
