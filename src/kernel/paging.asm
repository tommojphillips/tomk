; paging.asm
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;
; Paging using a statically allocated page directory and page tables.
; The page directory and all page tables occupy one contiguous 4 MiB + 4 KiB
; allocation, allowing page-table addresses to be derived directly from the
; page-directory base and PDE index without additional bookkeeping.
;

BITS 32

extern kalloc_page

global pd_base
global paging_init
global paging_enable
global paging_disable
global paging_map
global paging_flush
global paging_invalidate
global paging_get_physical

%include "src\kernel\include\common.inc"

P          equ 0x01            ; 0 = NP; 1 = Present
RW         equ 0x02            ; 0 = RO; 1 = RW
US         equ 0x04            ; 0 = Super; 1 = User
A          equ 0x20            ; 0 = not accessed; 1 = accessed
D          equ 0x40            ; 0 = not dirty; 1 = dirty

PAGE_SIZE  equ 0x1000          ; 4 KiB page size
PD_SIZE    equ PAGE_SIZE
PD_COUNT   equ 0x0400          ; Page directory count
PT_SIZE    equ PAGE_SIZE*PD_COUNT

Section .bss
    pd_base dd ?

Section .text

; Init page directory and page table
paging_init:
    push edx
    push edi

    push (PD_SIZE+PT_SIZE)
    call kalloc_page
    add esp, 4

    mov [pd_base], eax
    mov edx, eax               ; pd_base
    
    cld                        ; zero PD/PT (PD_BASE, 0, 4096+(4096*1024));
    xor eax, eax
    mov ecx, (PD_SIZE+PT_SIZE)/4
    mov edi, edx
    rep stosd

    mov eax, edx               ; pd_base
    mov cr3, eax               ; load page directory address into cr3    

    pop edi
    pop edx
    ret

; Enable paging
; Returns CR3
paging_enable:
    mov eax, cr0
    or eax, 0x80000000         ; PG bit
    mov cr0, eax

    jmp short .flush
.flush:
    ret

; Disable paging
; Returns CR3
paging_disable:
    mov eax, cr0
    and eax, 0x7FFFFFFF         ; PG bit
    mov cr0, eax

    jmp short .flush
.flush:
    ret

; map single page
; esp+8  = linear_address
; esp+12 = physical_address
; esp+16 = flags (lower 12bits)
_map_page:
    push ebx
    push edx
    push esi
    push edi

    mov edx, [pd_base]    
    mov esi, [esp+16+4]
    mov edi, [esp+16+8]
    mov ebx, [esp+16+12]

.loc_pde:
    push esi
    call paging_loc_pde
    add esp, 4

    test dword [eax], P
    jnz .loc_pte

.build_pde:
    mov ecx, esi               ; pd_index = linear_address >> 22
    shr ecx, 22
    shl ecx, 12
    add ecx, edx
    add ecx, PD_SIZE
    or ecx, US | RW | P        ; set flags
    mov [eax], ecx             ; write pde

.loc_pte:
    push esi
    call paging_loc_pte
    add esp, 4

.build_pte:                    ; build pte
    mov ecx, edi               ; get physical_address
    and ecx, 0xFFFFF000        ; clear lower 12bits
    and ebx, 0x00000FFF        ; clear upper 20bits in flags
    or ecx, ebx                ; set upper 20bits to physical_frame_address
    mov [eax], ecx             ; copy pte to memory 

.done:
    pop edi
    pop esi
    pop edx
    pop ebx
    ret

; map contiguous pages
; esp+4  = linear_address
; esp+8  = physical_address
; esp+12 = flags (lower 12bits)
; esp+16 = count (in 4096-byte units)
paging_map:
    push ebp                           ; save ebp
    mov ebp, esp                       ; save frame ptr
    add ebp, 4                         ; point frame ptr at params-4

    cmp dword [ebp+16], 0              ; count == zero?
    jz .done                           ; yes, dont map anything.

.nxt:
    push dword [ebp+12]                ; flags (lower 12bits)
    push dword [ebp+8]                 ; physical_address
    push dword [ebp+4]                 ; linear_address
    call _map_page
    add esp, 12

    add dword [ebp+4], PAGE_SIZE       ; linear_address += 4096
    add dword [ebp+8], PAGE_SIZE       ; physical_address += 4096
    dec dword [ebp+16]
    
    jnz .nxt

.done:
    pop ebp                            ; restore ebp
    ret

; flush entire TLB
; returns CR3
paging_flush:
    mov eax, cr3
    mov cr3, eax
    ret

; invalidate page
paging_invalidate:
    mov eax, cr3
    mov cr3, eax
    ret

; located PDE
; esp+4 = linear_address
; Returns pointer to PDE
paging_loc_pde:
    push edx
    push edi

    mov edx, [pd_base]
    mov edi, [esp+8+4]         ; linear_address

    ; compute pd_index
    mov eax, edi               ; linear_address >> 22
    shr eax, 22
    
    ; compute pde_address
    mov ecx, edx
    and ecx, 0xFFFFF000        ; pd_base clear lower 12bits
   
    lea eax, [ecx+eax*4]       ; pd_base + pd_index * 4

    pop edi
    pop edx
    ret

; located PTE
; esp+4 = linear_address
; Returns pointer to PTE
paging_loc_pte:
    push edx
    push edi

    mov edx, [pd_base]
    mov edi, [esp+8+4]         ; linear_address

    ; compute pt_index
    mov eax, edi
    shr eax, 12
    and eax, 0x3FF             ; pt_index = (linear_address >> 12) & 0x3FF

    ; compute pt_base
    mov ecx, edi
    shr ecx, 22
    shl ecx, 12
    add ecx, edx
    add ecx, PD_SIZE

    lea eax, [ecx+eax*4]       ; pt_base + pt_index * 4

    pop edi
    pop edx
    ret

; get physical address mapped to linear address
; esp+4 = linear_address
; returns physical address in eax
; returns 0 if not mapped
paging_get_physical:
    push esi

    mov esi, [esp+4+4]       ; linear_address
    xor ecx, ecx             ; ret_val
    
    push esi                 ; linear_address
    call paging_loc_pde      ; Locate PDE
    add esp, 4
    
    test dword [eax], P      ; PDE present?
    jz .done                 ; No, page not mapped; we done
    
    push esi                 ; linear_address     
    call paging_loc_pte      ; Locate PTE
    add esp, 4

    test dword [eax], P      ; PTE present?
    jz .done                 ; No, page not mapped; we done
    
    mov eax, [eax]           ; frame = pte   
    and eax, 0xFFFFF000      ; frame &= 0xFFFFF000;    
    mov ecx, esi             ; offset = linear_address;
    and ecx, 0x00000FFF      ; offset &= 0xFFF;
    or ecx, eax              ; phys_addr = frame | offset;

.done:
    pop esi    
    mov eax, ecx             ; return phys_addr;
    ret
