/* kernel/include/multiboot.h */

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

/* bit 0 in the 'flags' word is set, then the 'mem_*' fields are valid. 'mem_lower' and 
'mem_upper' indicate the amount of lower and upper memory, respectively, in kilobytes. 
Lower memory starts at address 0, and upper memory starts at address 1 megabyte. 
The maximum possible value for lower memory is 640 kilobytes. The value returned for 
upper memory is maximally the address of the first upper memory hole minus 1 megabyte. 
It is not guaranteed to be this value.*/
#define MULTIBOOT_FLAGS_MEM            0x00000001

/* bit 1 in the 'flags' word is set, then the 'boot_device' field is valid, and indicates 
which BIOS disk device the boot loader loaded the OS image from. If the OS image was not
loaded from a BIOS disk, then this field must not be present (bit 3 must be clear). 
The operating system may use this field as a hint for determining its own root device,
but is not required to. The 'boot_device' field is laid out in four one-byte subfields. */
#define MULTIBOOT_FLAGS_BOOT_DEVICE    0x00000002

/* If bit 2 of the ‘flags’ longword is set, the ‘cmdline’ field is valid, and contains 
the physical address of the command line to be passed to the kernel. The command line is
a normal C-style zero-terminated string. The exact format of command line is left to OS
developpers. General-purpose boot loaders should allow user a complete control on command
line independently of other factors like image name. Boot loaders with specific payload in
mind may completely or partially generate it algorithmically. */
#define MULTIBOOT_FLAGS_CMDLINE        0x00000004

typedef struct MULTIBOOT_INFO {
    uint32_t flags;
    uint32_t mem_lower;                          /* in kb. Lower memory starts at address 0 */
    uint32_t mem_upper;                          /* in kb. Upper memory starts at address 1 megabyte */
    uint8_t boot_device[4];
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t bootloader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint32_t vbe_mode;
    uint32_t vbe_interface_seg;
    uint32_t vbe_interface_off;
    uint32_t vbe_interface_len;
    uint32_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_bpp;
    uint32_t framebuffer_type;
    uint32_t color_info;
} MULTIBOOT_INFO;

#endif
