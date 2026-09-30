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

%include "src\include\kspacedef.inc"
%include "src\mm\include\paging.inc"
%include "src\init\include\mb.inc"

; %1: section start 
; %2: section end 
; %3: flags
; %4: physical start
; %5: virtual start
%macro map_section 5
    ; compute section size
    mov edx, %2
    sub edx, %1                                  ; compute size
    add edx, PAGE_SIZE-1                         ; page align end address
    shr edx, 12                                  ; get page count

    ; map section (KVIRT+1MB)
    push edx                                     ; page count
    push %3                                      ; flags
    push %4                                      ; physical address
    push %5                                      ; virtual address
    call map                                     ; map section (KVIRT)
    add esp, 16
%endmacro

section .bss
    mb_info_ptr dd ?    

section .boot progbits alloc exec nowrite

; Kernel Entrypoint
_start:
    cmp eax, 0x2BADB002                          ; multiboot v1 ?
    jnz .unk

.mb:
    add ebx, KVIRT                               ; convert pointer to a kernel virtual address
    mov [V2P(mb_info_ptr)], ebx                  ; save pointer
    jmp .bootstrap

.unk:
    mov dword [V2P(mb_info_ptr)], 0              ; NULL pointer

.bootstrap:

.map_pd:
    ; map all kernel PDE excluding the last pde, 1023
    mov edi, (KVIRT>>22)                         ; first kernel PDE
    mov ecx, 1023                                ; last kernel PDE
    sub ecx, edi                                 ; 1023 - 768 = 255
    
.lp:
    mov eax, edi
    shl eax, 12
    add eax, V2P(pg_pt)
    or eax, (RW | P)
    
    mov [V2P(pg_pd)+edi*4], eax
    
    inc edi
    dec ecx
    jnz .lp

.setup_recursive_mapping:
    ; PD/PT recursive mapping
    mov eax, V2P(pg_pd)
    or eax, (RW | P)
    mov [V2P(pg_pd)+1023*4], eax

.map_sections:
    
    ; map first 1MB to KVIRT
    map_section 0x1000, 0x100000, RW, 0x1000, P2V(0x1000)

    ; map .text section (KVIRT+1MB)
    map_section sec_text_start, sec_text_end, RO, V2P(sec_text_start), sec_text_start
    
    ; map .rodata section (KVIRT+1MB)
    map_section sec_rodata_start, sec_rodata_end, RO, V2P(sec_rodata_start), sec_rodata_start

    ; map .data section (KVIRT+1MB)
    map_section sec_data_start, sec_data_end, RW, V2P(sec_data_start), sec_data_start

    ; map .bss section (KVIRT+1MB)
    map_section sec_bss_start, sec_bss_end, RW, V2P(sec_bss_start), sec_bss_start

    ; map .boot section (identity)
    map_section sec_boot_start, sec_boot_end, RO, sec_boot_start, sec_boot_start

.enter_higher_half:

    lea ecx, [cs:kernel_init]

    ; load CR3
    mov eax, V2P(pg_pd)                          ; load physical address of page directory in CR3
    mov cr3, eax
    
    ; enable PG + WP
    mov eax, cr0
    or eax, (BIT_PG | BIT_WP)
    mov cr0, eax

    ; jump into the higher-half kernel
    jmp ecx

; map single page
; ebp+8  = virtual_address
; ebp+12 = physical_address
; ebp+16 = flags (lower 12bits)
_map:
    push ebp
    mov ebp, esp

    push ebx
    push esi
    push edi

    mov esi, [ebp+8]                             ; virtual_address
    mov edi, [ebp+12]                            ; physical_address
    mov ebx, [ebp+16]                            ; flags

.loc_pde:
    mov eax, esi
    shr eax, 22                                  ; pd_index = (virtual_address >> 22)
    lea eax, [V2P(pg_pd)+eax*4]                  ; pd_base + pd_index * 4

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
    mov eax, esi
    shr eax, 12
    and eax, 0x3FF                               ; pt_index = (virtual_address >> 12) & 0x3FF
    mov ecx, esi
    shr ecx, 10
    and ecx, 0xFFFFF000                          ; pt_offset = ((virtual_address >> 10) & 0xFFFFF000)
    lea eax, [V2P(pg_pt)+ecx+eax*4]              ; pte = (pt_base + pt_offset + pt_index * 4)

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
    leave
    ret

; map contiguous pages
; ebp+8  = virtual_address
; ebp+12  = physical_address
; ebp+16 = flags (lower 12bits)
; ebp+20 = count (in 4096-byte units)
map:
    push ebp
    mov ebp, esp

    cmp dword [ebp+20], 0                        ; count == zero?
    jz .done                                     ; yes, dont map any pages

.nxt:
    push dword [ebp+16]                          ; flags (lower 12bits)
    push dword [ebp+12]                          ; physical_address
    push dword [ebp+8]                           ; virtual_address
    call _map                                    ; map page
    add esp, 12

    add dword [ebp+8], PAGE_SIZE                 ; virtual_address += 4096
    add dword [ebp+12], PAGE_SIZE                ; physical_address += 4096
    dec dword [ebp+20]                           ; count -= 1        
    jnz .nxt                                     ; count > 0?

.done:
    leave
    ret
