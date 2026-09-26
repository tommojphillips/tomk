; common.inc
; Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
;

extern gdt_selector_ke_code            ; Kernelspace code segment
extern gdt_selector_ke_data            ; Kernelspace data segment
extern gdt_selector_ke_stack           ; Kernelspace stack segment
extern gdt_selector_ke_tss             ; Kernelspace tss

extern gdt_selector_user_code          ; Userspace code segment
extern gdt_selector_user_data          ; Userspace data segment
extern gdt_selector_user_stack         ; Userspace stack segment
