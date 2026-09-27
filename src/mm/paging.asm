; paging.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; Paging using a statically allocated page directory and page tables.
; The page directory and all page tables occupy one contiguous 4 MiB + 4 KiB
; allocation, allowing page-table addresses to be derived directly from the
; page-directory base and PDE index without additional bookkeeping.
;

BITS 32

; Debugging
%define PG_DBG

%ifdef PG_DBG
extern kdprintf
%endif

global pg_map
global pg_unmap
global pg_flush
global pg_invalidate
global pg_virt2phys
global pg_chgpriv
global pg_pd
global pg_pt

%include "src\include\kspacedef.inc"
%include "src\mm\include\paging.inc"

PD_VIRT equ 0xFFFFF000
PT_VIRT equ 0xFFC00000

; Page directory / Page tables
Section .bss
    align 4096, db 0
pg_pd:
    resb 4096
pg_pt:
    resb 4096*1024

%ifdef PG_DBG
Section .rodata
    dbg_map_str db "[PG] map cr3=%08X virt=%08X phys=%08X flags=%02X count=%u", 10, 0
    dbg_unmap_str db "[PG] unmap cr3=%08X virt=%08X count=%u", 10, 0
    dbg_chgpriv_str db "[PG] chgpriv cr3=%08X virt=%08X flags=%02X count=%u", 10, 0
%endif

Section .text

; map single page
; ebp+8  = virtual_address
; ebp+12 = physical_address
; ebp+16 = flags (lower 12bits)
_map_page:
    push ebp
    mov ebp, esp

    push ebx
    push esi
    push edi

    mov esi, [ebp+8]                             ; virtual_address
    mov edi, [ebp+12]                            ; physical_address
    mov ebx, [ebp+16]                            ; flags

.loc_pde:
    push esi                   
    call pg_loc_pde                              ; get pde pointer
    add esp, 4

    test dword [eax], P                          ; pde present?
    jnz .loc_pte                                 ; yes, skip building pde

.build_pde:
    mov ecx, esi                                 ; pde = virtual_address
    shr ecx, 10                                  ; pde >>= 10
    and ecx, 0xFFFFF000                          ; pde &= 0xFFFFF000
    add ecx, V2P(pg_pt)                          ; pde += pg_pt (physical address)
    or ecx, (RW | P)                             ; pde |= (PTE_P | PTE_RW)
    mov [eax], ecx                               ; write pde

.loc_pte:
    push esi
    call pg_loc_pte                              ; get pte pointer
    add esp, 4

.build_pte:
    mov ecx, edi                                 ; physical_address
    and ecx, 0xFFFFF000                          ; get physical_page_frame
    and ebx, (PAGE_SIZE-1)                       ; get flags
    or ebx, P                                    ; set present bit
    or ecx, ebx                                  ; set flags
    mov [eax], ecx                               ; write pte
    invlpg [esi]                                 ; invalidate TLB entry

.done:
    pop edi
    pop esi
    pop ebx
    leave
    ret

; unmap single page
; ebp+8  = virtual_address
_unmap_page:
    push ebp
    mov ebp, esp

    push esi

    mov esi, [ebp+8]                             ; virtual_address

.loc_pde:
    push esi                   
    call pg_loc_pde                              ; get pde pointer
    add esp, 4

    test dword [eax], P                          ; pde present?
    jz .done                                     ; no, done

.loc_pte:
    push esi
    call pg_loc_pte                              ; get pte pointer
    add esp, 4

    test dword [eax], P                          ; pte present?
    jz .done                                     ; no, done
    
    mov dword [eax], NP                          ; write pte
    invlpg [esi]                                 ; invalidate TLB entry

.done:
    pop esi
    leave
    ret

; map contiguous pages
; ebp+8  = virtual_address
; ebp+12 = physical_address
; ebp+16 = flags (lower 12bits)
; ebp+20 = count (in 4096-byte units)
pg_map:
    push ebp
    mov ebp, esp

%ifdef PG_DBG
    mov eax, cr3
    push [ebp+20]
    push [ebp+16]
    push [ebp+12]
    push [ebp+8]
    push eax
    push dbg_map_str
    call kdprintf
    add esp, 24
%endif

    cmp dword [ebp+20], 0                        ; count == zero?
    jz .done                                     ; yes, dont map any pages

.nxt:                                            ; map next page
    push dword [ebp+16]                          ; flags (lower 12bits)
    push dword [ebp+12]                          ; physical_address
    push dword [ebp+8]                           ; virtual_address
    call _map_page                               ; map page
    add esp, 12

    add dword [ebp+8], PAGE_SIZE                 ; virtual_address += 4096
    add dword [ebp+12], PAGE_SIZE                ; physical_address += 4096
    dec dword [ebp+20]                           ; count -= 1    
    jnz .nxt                                     ; count > 0?

.done:
    leave
    ret

; unmap contiguous pages
; ebp+8  = virtual_address
; ebp+12 = count (in 4096-byte units)
pg_unmap:
    push ebp
    mov ebp, esp

    push esi
    push ebx

    mov esi, [ebp+8]
    mov ebx, [ebp+12]

%ifdef PG_DBG
    mov eax, cr3
    push ebx
    push esi
    push eax
    push dbg_unmap_str
    call kdprintf
    add esp, 16
%endif

    test ebx, ebx                                ; count == zero?
    jz .done                                     ; yes, dont map any pages

.nxt:                                            ; map next page
    push esi                                     ; virtual_address
    call _unmap_page                             ; unmap page
    add esp, 4

    add esi, PAGE_SIZE                           ; virtual_address += 4096
    dec ebx                                      ; count -= 1    
    jnz .nxt                                     ; count > 0?

.done:
    pop ebx
    pop esi
    leave
    ret

; flush entire TLB
pg_flush:
    push ebp
    mov ebp, esp

    mov eax, cr3
    mov cr3, eax                                 ; reloading CR3 invalidates all TLB entries
    
    leave
    ret

; invalidate pages
; ebp+8  = virtual_address
; ebp+12 = page count
pg_invalidate:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]                             ; virtual_address
    and eax, 0xFFFFF000                          ; page align virtual_address
    mov ecx, [ebp+12]                            ; page_count

    test ecx, ecx                                ; page_count == 0?
    jz .done                                     ; yes, dont invalidate anything

.lp:
    invlpg [eax]                                 ; invalidate page in TLB
    add eax, PAGE_SIZE
    dec ecx
    jnz .lp
.done:
    leave
    ret

; Get pde (virtual) pointer
; ebp+8 = virtual_address
; Returns pointer to PDE
pg_loc_pde:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]                             ; virtual_address
    shr eax, 22                                  ; compute pd_index
    lea eax, [PD_VIRT+eax*4]                     ; pde = pd_base + pd_index * 4
    
    leave
    ret

; Get pte (virtual) pointer
; ebp+8 = virtual_address
; Returns pointer to PTE
pg_loc_pte:
    push ebp
    mov ebp, esp

    mov ecx, [ebp+8]                             ; virtual_address

    ; compute pt_index
    mov eax, ecx
    shr eax, 12
    and eax, 0x3FF                               ; pt_index = (virtual_address >> 12) & 0x3FF

    ; compute pt_offset
    shr ecx, 10
    and ecx, 0xFFFFF000                          ; pt_offset = ((virtual_address >> 10) & 0xFFFFF000)

    lea eax, [PT_VIRT+ecx+eax*4]                 ; pte = pt_base + pt_offset + pt_index * 4

    leave
    ret

; get physical address mapped to linear address
; ebp+8 = virtual_address
; returns physical address in eax
; returns 0 if not mapped
pg_virt2phys:
    push ebp
    mov ebp, esp

    push esi

    mov esi, [ebp+8]                             ; virtual_address
    
    push esi                                     ; virtual_address
    call pg_loc_pde                              ; get pde pointer
    add esp, 4
    
    test dword [eax], P                          ; pde present?
    jz .err                                      ; no, page not mapped; done
    
    push esi                                     ; virtual_address     
    call pg_loc_pte                              ; get pte pointer
    add esp, 4

    test dword [eax], P                          ; pte present?
    jz .err                                      ; no, page not mapped; done
    
    mov eax, [eax]                               ; pte
    and eax, 0xFFFFF000                          ; get page_frame
    mov ecx, esi                                 ; virtual_address
    and ecx, (PAGE_SIZE-1)                       ; get page_offset
    or ecx, eax                                  ; set phys_addr; (page_frame | page_offset)

.ok:
    mov eax, ecx                                 ; return phys_addr
    jmp .done
.err:
    xor eax, eax                                 ; return 0 (error)
.done:
    pop esi
    leave
    ret

; change access permissions
; ebp+8  = virtual_address
; ebp+12  = flags (RW and US bits)
; ebp+16 = count
pg_chgpriv:
    push ebp
    mov ebp, esp

    push esi
    push ebx
    push edx

    mov esi, [ebp+8]
    mov ebx, [ebp+12]
    mov edx, [ebp+16]

%ifdef PG_DBG
    mov eax, cr3
    push edx
    push ebx
    push esi
    push eax
    push dbg_chgpriv_str
    call kdprintf
    add esp, 20
%endif

    and ebx, (RW | US)                           ; only keep RW/US

    test edx, edx                                ; page_count == 0?
    jz .done                                     ; yes, done

.lp:
    push esi                                     ; virtual_address
    call pg_loc_pte                              ; get pte pointer
    add esp, 4

    and [eax], ~(RW | US)                        ; clear RW/US bits in pte
    or [eax], ebx                                ; set RW/US bits in pte to flags
    
    invlpg [esi]
    
    add esi, PAGE_SIZE
    dec edx
    jnz .lp

.done:
    pop edx
    pop ebx
    pop esi
    leave
    ret
