BITS 32

global paging_init
global paging_enable
global paging_disable
global paging_map_page
global paging_map_pages
global paging_map_bytes

%include "src\kernel\include\common.inc"

section .text

paging_init:
    
    ; map virt:0x00000000-0x01000000 -> phys:0x00000000-0x01000000 R/W Ring0 (16Mb)
    mov edx, PD_BASE
    mov esi, 0
    mov edi, 0
    mov eax, 3
    mov ecx, 0x1000000
    call paging_map_pages
    
    ; load page directory address into cr3
    mov cr3, edx

    call paging_enable

    ret

; Enable paging
paging_enable:
    mov eax, cr0
    or eax, 0x80000000         ; PG bit
    mov cr0, eax

    jmp short .flush
.flush:
    ret

; Disable paging
paging_disable:
    mov eax, cr0
    and eax, 0x7FFFFFFF         ; PG bit
    mov cr0, eax

    jmp short .flush
.flush:
    ret

; map single page
; edx = pd_base
; esi = physical_address
; edi = linear_address
; eax = flags (lower 12bits)
paging_map_page:
    push edx                   ; save pd_base
    push esi                   ; save physical_address
    push edi                   ; save linear_address
    push eax                   ; save flags

    push eax                   ; save flags
    push esi                   ; save physical_address

    ; compute pd_index
    mov ebx, edi               ; linear_address >> 22
    shr ebx, 22
    
    ; compute pde_address
    mov esi, edx               ; PDBR 
    and esi, 0FFFFF000h        ; PDBR clear lower 12bits
    mov eax, edi               ; linear_address
    shr eax, 22                ; linear_address >> 22
    lea esi, [esi+eax*4]

    ; compute pt_index
    mov edx, edi               ; (linear_address >> 12) & 1023
    shr edx, 12
    and edx, 03FFh
    
    ; compute pte_address
    mov edi, ebx               ; pt_base + (pd_index * 4096)
    shl edi, 12
    add edi, PT_BASE    

    mov eax, [esi]             ; get pde
    test eax, 1                ; pde present?
    jnz .have_pde              ; yes, dont rebuild pde
;                              ; no, build pde
    mov eax, edi               ; pte_address
    or  eax, 3                 ; set present bit
    mov [esi], eax             ; set pde

.have_pde:

    ; build pte
    pop esi                    ; restore physical_address
    and esi, 0FFFFF000h        ; clear lower 12bits    
    pop eax                    ; restore flags
    and eax, 00000FFFh         ; clear upper 20bits
    or  eax, esi               ; set upper 12bits to physical_address
    mov [edi+edx*4], eax       ; copy pte to memory 
    
    pop eax                    ; restore flags
    pop edi                    ; restore linear_address
    pop esi                    ; restore physical_address
    pop edx                    ; restore pd_base

    ret

; map contiguous pages
; edx = pd_base
; esi = start physical_address
; edi = start linear_address
; eax = flags (lower 12bits)
; ecx = end physical address (exclusive) (increments of 4096 bytes)
paging_map_pages:
.nxt:
    call map_page
    add esi, PAGE_SIZE
    add edi, PAGE_SIZE
    cmp esi, ecx
    jne .nxt
    ret

; map contiguous pages
; edx = pd_base
; esi = start physical_address
; edi = start linear_address
; eax = flags (lower 12bits)
; ecx = count (in bytes)
paging_map_bytes:
    test ebx, 0xFFF
    jz .skip
    add ecx, 0x1000
.skip:
    shr ecx, 12

    call map_block
    ret
