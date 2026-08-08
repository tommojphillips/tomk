BITS 32

global paging_init
global paging_enable
global paging_disable
global paging_map_page
global paging_map_pages
global paging_map_bytes

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
    jnz .build_pte             ; yes, skip pde

.build_pde:                    ; no, build pde
    ;mov eax, [esp+12+8]        ; get flags
    ;and eax, 00000FBFh         ; clear upper 20bits + dirty bit
    ;or eax, edi                ; pte_address
    mov eax, edi               ; pte address
    or eax, 1                  ; set present bit
    mov [esi], eax             ; set pde

.build_pte:                    ; build pte
    mov esi, [esp+12+16]       ; get physical_address
    and esi, 0FFFFF000h        ; clear lower 12bits    
    mov eax, [esp+12+8]        ; get flags
    and eax, 00000FFFh         ; clear upper 20bits
    or eax, esi                ; set upper 12bits to physical_address
    mov [edi+edx*4], eax       ; copy pte to memory 

.done:
    pop ebx
    pop edi
    pop esi
    pop edx
    ret

; map contiguous pages
; edx = pd_base
; esi = start physical_address
; edi = start linear_address
; eax = flags (lower 12bits)
; ecx = count (in 4096-byte units)
paging_map_pages:
    push edi
    push esi
    push edx

    mov ecx, [esp+12+4]
    mov eax, [esp+12+8]
    mov edi, [esp+12+12]
    mov esi, [esp+12+16]
    mov edx, [esp+12+20]

    test ecx, ecx
    jz .done

.nxt:
    push ecx
    push edx
    push esi
    push edi
    push eax
    call paging_map_page
    add esp, 16
    pop ecx

    add esi, PAGE_SIZE
    add edi, PAGE_SIZE
    dec ecx
    jnz .nxt

.done:
    pop edx
    pop esi
    pop edi
    ret

; map contiguous pages
; edx = pd_base
; esi = start physical_address
; edi = start linear_address
; eax = flags (lower 12bits)
; ecx = count (in bytes)
paging_map_bytes:
    push edi
    push esi
    push edx

    mov ecx, [esp+12+4]
    mov eax, [esp+12+8]
    mov edi, [esp+12+12]
    mov esi, [esp+12+16]
    mov edx, [esp+12+20]

    test ecx, 0x00000FFF ; page offset?
    jz .skip             ; yes, skip inc
    add ecx, PAGE_SIZE   ; no, add page
.skip:
    shr ecx, 12          ; compute page count

    push edx
    push esi
    push edi
    push eax
    push ecx
    call paging_map_pages
    add esp, 20

    ret
