; paging.asm

BITS 32

global paging_init
global paging_enable
global paging_disable
global paging_map
global paging_flush

%include "src\kernel\include\common.inc"

P          equ 0x01            ; 0 = NP; 1 = Present
RW         equ 0x02            ; 0 = RO; 1 = RW
US         equ 0x04            ; 0 = Super; 1 = User
A          equ 0x20            ; 0 = not accessed; 1 = accessed
D          equ 0x40            ; 0 = not dirty; 1 = dirty

PAGE_SIZE  equ 0x1000          ; Page table size
PD_SIZE    equ PAGE_SIZE
PD_COUNT   equ 0x0400          ; Page directory count

section .text

paging_init:
    push edx
    push edi

    mov edx, [esp+8+4]         ; pd_base
    
    cld                        ; zero PD (PD_BASE, 0, 4096);
    xor eax, eax
    mov ecx, PAGE_SIZE/4
    mov edi, edx
    rep stosd
    
    mov ecx, (PAGE_SIZE*PD_COUNT)/4 ; zero PT (PT_BASE, 0, 4096*1024);
    mov edi, edx
    add edi, PAGE_SIZE
    rep stosd

    mov cr3, edx               ; load page directory address into cr3    

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
; edx = pd_base
; esi = physical_address
; edi = linear_address
; eax = flags (lower 12bits)
_map_page:
    push edx
    push esi
    push edi
    push ebx

    mov eax, [esp+16+4]
    mov edi, [esp+16+8]
    mov esi, [esp+16+12]
    mov edx, [esp+16+16]

    ; compute pd_index
    mov ebx, edi               ; linear_address >> 22
    shr ebx, 22
    
    ; compute pde_address
    mov esi, edx               ; PDBR 
    and esi, 0xFFFFF000        ; PDBR clear lower 12bits
    lea esi, [esi+ebx*4]

    ; compute pt_index
    mov edx, edi               ; (linear_address >> 12) & 1023
    shr edx, 12
    and edx, 0x3FF
    
    ; compute pte_address
    mov edi, ebx               ; (pd_index * 4096) + pd_base + pd_size 
    shl edi, 12
    add edi, [esp+16+16]
    add edi, PD_SIZE

    mov eax, [esi]             ; get pde
    test eax, P                ; pde present?
    jnz .build_pte             ; yes, skip pde

.build_pde:                    ; no, build pde
    mov eax, edi               ; pte address
    or eax, US | RW | P        ; set present bit
    mov [esi], eax             ; set pde

.build_pte:                    ; build pte
    mov esi, [esp+12+16]       ; get physical_address
    and esi, 0xFFFFF000        ; clear lower 12bits    
    mov eax, [esp+12+8]        ; get flags
    and eax, 0x00000FFF        ; clear upper 20bits
    or eax, esi                ; set upper 20bits to physical_frame_address
    mov [edi+edx*4], eax       ; copy pte to memory 

.done:
    pop ebx
    pop edi
    pop esi
    pop edx
    ret

; map contiguous pages
; edx = pd_base
; esi = start linear_address
; edi = start physical_address
; eax = flags (lower 12bits)
; ecx = count (in 4096-byte units)
paging_map:
    push edi
    push esi
    push edx
    push ebx

    mov ecx, [esp+16+4]                ; page count (in 4096-byte units)
    mov eax, [esp+16+8]                ; flags (lower 12bits)
    mov edi, [esp+16+12]               ; start physical_address
    mov esi, [esp+16+16]               ; start linear_address
    mov edx, [esp+16+20]               ; pd_base

    test ecx, ecx                      ; count == zero?
    jz .done                           ; yes, dont map anything.

.nxt:
    push ecx                           ; save page count
    push edx
    push esi
    push edi
    push eax
    call _map_page
    add esp, 16
    pop ecx                           ; restore page count
    
    add esi, PAGE_SIZE                ; linear_address += 4096
    add edi, PAGE_SIZE                ; physical_address += 4096
    dec ecx
    jnz .nxt

.done:
    pop ebx
    pop edx
    pop esi
    pop edi
    ret

; flush TLB
; returns CR3
paging_flush:
    mov eax, cr3
    mov cr3, eax
    ret
