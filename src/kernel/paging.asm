; paging.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; Paging using a statically allocated page directory and page tables.
; The page directory and all page tables occupy one contiguous 4 MiB + 4 KiB
; allocation, allowing page-table addresses to be derived directly from the
; page-directory base and PDE index without additional bookkeeping.
;

BITS 32

global pg_map
global pg_unmap
global pg_flush
global pg_invalidate
global pg_virt2phys
global pg_chgpriv
global pg_pd
global pg_pt

%include "src\kernel\include\common.inc"
%include "src\kernel\include\paging.inc"

; Page directory / Page tables
Section .bss
    align 4096, db 0
pg_pd:
    resb 4096
pg_pt:
    resb 4096*1024

Section .text

; map single page
; esp+4  = virtual_address
; esp+8  = physical_address
; esp+12 = flags (lower 12bits)
_map_page:
    push ebx
    push esi
    push edi

    mov esi, [esp+12+4]                          ; virtual_address
    mov edi, [esp+12+8]                          ; physical_address
    mov ebx, [esp+12+12]                         ; flags

.loc_pde:
    push esi                   
    call pg_loc_pde                              ; get PDE pointer in EAX
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
    call pg_loc_pte                              ; get PTE pointer in EAX
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

; unmap single page
; esp+4  = virtual_address
_unmap_page:
    push esi

    mov esi, [esp+4+4]                          ; virtual_address

.loc_pde:
    push esi                   
    call pg_loc_pde                              ; get PDE pointer in EAX
    add esp, 4

    test dword [eax], P                          ; pde present?
    jz .done                                     ; no, done

.loc_pte:
    push esi
    call pg_loc_pte                              ; get PTE pointer in EAX
    add esp, 4

    test dword [eax], P                          ; pte present?
    jz .done                                     ; no, done
    
.build_pte:
    mov [eax], NP                                ; write pte

.done:
    pop esi
    ret

; map contiguous pages
; esp+4  = virtual_address
; esp+8  = physical_address
; esp+12 = flags (lower 12bits)
; esp+16 = count (in 4096-byte units)
pg_map:
    push ebp                                     ; save ebp
    mov ebp, esp                                 ; save frame ptr
    add ebp, 4                                   ; point frame ptr at params-4

    cmp dword [ebp+16], 0                        ; count == zero?
    jz .done                                     ; yes, dont map any pages

.nxt:                                            ; map next page
    push dword [ebp+12]                          ; flags (lower 12bits)
    push dword [ebp+8]                           ; physical_address
    push dword [ebp+4]                           ; virtual_address
    call _map_page                               ; map page
    add esp, 12

    add dword [ebp+4], PAGE_SIZE                 ; virtual_address += 4096
    add dword [ebp+8], PAGE_SIZE                 ; physical_address += 4096
    dec dword [ebp+16]                           ; count -= 1    
    jnz .nxt                                     ; count > 0?

.done:
    pop ebp                                      ; restore ebp
    ret

; unmap contiguous pages
; esp+4 = virtual_address
; esp+8 = count (in 4096-byte units)
pg_unmap:
    push esi
    push ebx

    mov esi, [esp+8+4]
    mov ebx, [esp+8+8]

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
    ret

; flush entire TLB
pg_flush:
    mov eax, cr3
    mov cr3, eax                                 ; reloading CR3 invalidates all TLB entries
    ret

; invalidate pages
; esp+4 = virtual_address
; esp+8 = page count
pg_invalidate:
    mov eax, [esp+4]                             ; virtual_address
    and eax, 0xFFFFF000                          ; page align virtual_address
    mov ecx, [esp+8]                             ; page_count

    test ecx, ecx                                ; page_count == 0?
    jz .done                                     ; yes, dont invalidate anything

.lp:
    invlpg [eax]                                 ; invalidate page in TLB
    add eax, PAGE_SIZE
    dec ecx
    jnz .lp
.done:
    ret

; located PDE
; esp+4 = virtual_address
; Returns pointer to PDE
pg_loc_pde:
    mov eax, [esp+4]                             ; virtual_address
    shr eax, 22                                  ; compute pd_index
    lea eax, [pg_pd+eax*4]                       ; pde = pd_base + pd_index * 4
    ret

; located PTE
; esp+4 = virtual_address
; Returns pointer to PTE
pg_loc_pte:
    mov ecx, [esp+4]                             ; virtual_address

    ; compute pt_index
    mov eax, ecx
    shr eax, 12
    and eax, 0x3FF                               ; pt_index = (virtual_address >> 12) & 0x3FF

    ; compute pt_offset
    shr ecx, 10
    and ecx, 0xFFFFF000                          ; pt_offset = ((virtual_address >> 10) & 0xFFFFF000)

    lea eax, [pg_pt+ecx+eax*4]                   ; pte = pt_base + pt_offset + pt_index * 4
    ret

; get physical address mapped to linear address
; esp+4 = virtual_address
; returns physical address in eax
; returns 0 if not mapped
pg_virt2phys:
    push esi

    mov esi, [esp+4+4]                           ; virtual_address
    xor ecx, ecx                                 ; ret_val
    
    push esi                                     ; virtual_address
    call pg_loc_pde                              ; locate pde
    add esp, 4
    
    test dword [eax], P                          ; pde present?
    jz .done                                     ; no, page not mapped; done
    
    push esi                                     ; virtual_address     
    call pg_loc_pte                              ; locate pte
    add esp, 4

    test dword [eax], P                          ; pte present?
    jz .done                                     ; no, page not mapped; done
    
    mov eax, [eax]                               ; pte
    and eax, 0xFFFFF000                          ; get page_frame
    mov ecx, esi                                 ; virtual_address
    and ecx, (PAGE_SIZE-1)                       ; get page_offset
    or ecx, eax                                  ; set phys_addr; (page_frame | page_offset)

.done:
    pop esi
    mov eax, ecx                                 ; return phys_addr
    ret

; change access permissions
; esp+4  = virtual_address
; esp+8  = flags (RW and US bits)
; esp+12 = count
pg_chgpriv:
    push esi
    push ebx
    push edx

    mov esi, [esp+12+4]
    mov ebx, [esp+12+8]
    mov edx, [esp+12+12]

    and ebx, (RW | US)                           ; only keep RW/US

    test edx, edx                                ; page_count == 0?
    jz .done                                     ; yes, done

.lp:
    push esi                                     ; virtual_address
    call pg_loc_pte                              ; locate pte
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
    ret
