; entry.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; Kernel Entry
;

BITS 32

extern sec_boot_start                            ; linker.ld
extern sec_boot_end                              ; linker.ld

extern sec_bss_start                             ; linker.ld
extern sec_bss_end                               ; linker.ld

extern sec_text_start                            ; linker.ld
extern sec_text_end                              ; linker.ld

extern sec_rodata_start                          ; linker.ld
extern sec_rodata_end                            ; linker.ld

extern sec_data_start                            ; linker.ld
extern sec_data_end                              ; linker.ld

extern sec_bss_start                             ; linker.ld
extern sec_bss_end                               ; linker.ld

extern kernel_init                               ; kernel.asm
extern pg_pd                                     ; paging.asm
extern pg_pt                                     ; paging.asm
global mb_info_ptr
global _start

%include "src\kernel\include\common.inc"
%include "src\kernel\include\paging.inc"

section .bss
    mb_info_ptr dd ?    

section .boot

_start:
    cmp eax, 0x2BADB002                          ; multiboot v1 ?
    jnz .unk

.mb:
    add ebx, KVIRT                               ; convert pointer to a kernel virtual address
    mov [V2P(mb_info_ptr)], ebx                  ; save pointer
    jmp .enter_higher_half

.unk:
    mov dword [V2P(mb_info_ptr)], 0              ; NULL pointer

.enter_higher_half:

    ; map first 1MB to KVIRT
    push 256-1                                   ; page count
    push RW                                      ; flags
    push 0x00001000                              ; physical address
    push 0x00001000+KVIRT                        ; virtual address
    call map                                     ; map first 1MB to KVIRT
    add esp, 16

    ; compute .text section size
    mov edx, sec_text_end
    add edx, 0xFFF                               ; page align end address
    sub edx, sec_text_start                      ; compute size
    shr edx, 12                                  ; get page count

    ; map .text section (KVIRT+1MB) (Read-Only)
    push edx                                     ; page count
    push RO                                      ; flags
    push V2P(sec_text_start)                     ; physical address
    push sec_text_start                          ; virtual address
    call map                                     ; map .text section (KVIRT)
    add esp, 16
    
    ; compute .rodata section size
    mov edx, sec_rodata_end
    add edx, 0xFFF                               ; page align end address
    sub edx, sec_rodata_start                    ; compute size
    shr edx, 12                                  ; get page count  

    ; map .rodata section (KVIRT+1MB) (Read-Only)
    push edx                                     ; page count
    push RO                                      ; flags
    push V2P(sec_rodata_start)                   ; physical address
    push sec_rodata_start                        ; virtual address
    call map                                     ; map .rodata section (KVIRT)
    add esp, 16

    ; compute .data section size
    mov edx, sec_data_end
    add edx, 0xFFF                               ; page align end address
    sub edx, sec_data_start                      ; compute size
    shr edx, 12                                  ; get page count  

    ; map .data section (KVIRT+1MB) (Read-Write)
    push edx                                     ; page count
    push RW                                      ; flags
    push V2P(sec_data_start)                     ; physical address
    push sec_data_start                          ; virtual address
    call map                                     ; map .data section (KVIRT)
    add esp, 16

    ; compute .bss section size
    mov edx, sec_bss_end
    add edx, 0xFFF                               ; page align end address
    sub edx, sec_bss_start                       ; compute size
    shr edx, 12                                  ; get page count  

    ; map .bss section (KVIRT+1MB) (Read-Write)
    push edx                                     ; page count
    push RW                                      ; flags
    push V2P(sec_bss_start)                      ; physical address
    push sec_bss_start                           ; virtual address
    call map                                     ; map .bss section (KVIRT)
    add esp, 16
        
    ; compute .boot section size
    mov edx, sec_boot_end
    add edx, 0xFFF                               ; page align end address
    sub edx, sec_boot_start                      ; compute size
    shr edx, 12                                  ; get page count

    ; map .boot section (1MB identity) (Read-Only)
    push edx                                     ; page count
    push RO                                      ; flags
    push sec_boot_start                          ; physical address
    push sec_boot_start                          ; virtual address
    call map                                     ; map .boot section (identity)
    add esp, 16

    ; load CR3
    mov eax, V2P(pg_pd)                          ; load physical address of page directory in CR3
    mov cr3, eax
    
    ; Enable PG + WP
    mov eax, cr0
    or eax, (BIT_PG | BIT_WP)
    mov cr0, eax

    ; jump into the higher-half kernel
    lea eax, [cs:kernel_init]
    jmp eax

; located PDE
; esp+4 = virtual_address
; Returns pointer to PDE
loc_pde:
    mov eax, [esp+4]                             ; virtual_address
    shr eax, 22                                  ; compute pd_index
    lea eax, [V2P(pg_pd)+eax*4]                  ; pd_base + pd_index * 4
    ret

; located PTE
; esp+4 = virtual_address
; Returns pointer to PTE
loc_pte:
    mov ecx, [esp+4]                             ; virtual_address
    mov eax, ecx                                 ; compute pt_index
    shr eax, 12
    and eax, 0x3FF                               ; pt_index = (virtual_address >> 12) & 0x3FF
    shr ecx, 10                                  ; compute pt_offset
    and ecx, 0xFFFFF000                          ; pt_offset = ((virtual_address >> 10) & 0xFFFFF000)
    lea eax, [V2P(pg_pt)+ecx+eax*4]              ; pt_base + pt_offset + pt_index * 4
    ret

; map single page
; esp+4  = virtual_address
; esp+8  = physical_address
; esp+12 = flags (lower 12bits)
_map:
    push ebx
    push esi
    push edi

    mov esi, [esp+12+4]                          ; virtual_address
    mov edi, [esp+12+8]                          ; physical_address
    mov ebx, [esp+12+12]                         ; flags

.loc_pde:
    push esi                   
    call loc_pde                                 ; get PDE pointer in EAX
    add esp, 4

    test dword [eax], P                          ; pde present?
    jnz .loc_pte                                 ; yes, skip building pde

.build_pde:
    mov ecx, esi                                 ; pde = virtual_address
    shr ecx, 10                                  ; pde >>= 10
    and ecx, 0xFFFFF000                          ; pde &= 0xFFFFF000
    add ecx, V2P(pg_pt)                          ; pde += pg_pt
    or ecx, (RW | P)                             ; pde |= (PTE_P | PTE_RW)
    mov [eax], ecx                               ; write pde

.loc_pte:
    push esi
    call loc_pte                                 ; get PTE pointer in EAX
    add esp, 4

.build_pte:
    mov ecx, edi                                 ; physical_address
    and ecx, 0xFFFFF000                          ; get physical_page_frame
    and ebx, (PAGE_SIZE-1)                       ; get flags
    or ebx, P                                    ; set present bit
    or ecx, ebx                                  ; set flags
    mov [eax], ecx                               ; write pte

.done:
    pop edi
    pop esi
    pop ebx
    ret

; map contiguous pages
; esp+4  = virtual_address
; esp+8  = physical_address
; esp+12 = flags (lower 12bits)
; esp+16 = count (in 4096-byte units)
map:
    push ebp                                     ; save ebp
    mov ebp, esp                                 ; save frame ptr
    add ebp, 4                                   ; point frame ptr at params-4

    cmp dword [ebp+16], 0                        ; count == zero?
    jz .done                                     ; yes, dont map any pages

.nxt:
    push dword [ebp+12]                          ; flags (lower 12bits)
    push dword [ebp+8]                           ; physical_address
    push dword [ebp+4]                           ; virtual_address
    call _map                                    ; map page
    add esp, 12

    add dword [ebp+4], PAGE_SIZE                 ; virtual_address += 4096
    add dword [ebp+8], PAGE_SIZE                 ; physical_address += 4096
    dec dword [ebp+16]                           ; count -= 1        
    jnz .nxt                                     ; count > 0?

.done:
    pop ebp                                      ; restore ebp
    ret
