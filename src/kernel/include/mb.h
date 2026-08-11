/* multiboot.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

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

/* If bit 2 of the 'flags' longword is set, the 'cmdline' field is valid, and contains 
the physical address of the command line to be passed to the kernel. The command line is
a normal C-style zero-terminated string. The exact format of command line is left to OS
developpers. General-purpose boot loaders should allow user a complete control on command
line independently of other factors like image name. Boot loaders with specific payload in
mind may completely or partially generate it algorithmically. */
#define MULTIBOOT_FLAGS_CMDLINE        0x00000004

/* bit 5 in the 'flags' word is set, then the following fields in the Multiboot information 
structure starting at byte 28. These indicate where the section header table from an ELF
kernel is, the size of each entry, number of entries, and the string table used as the index
of names. They correspond to the shdr_* entries (shdr_num, etc.) in the Executable and Linkable
Format (ELF) specification in the program header. All sections are loaded, and the physical 
address fields of the ELF section header then refer to where the sections are in memory (refer 
to the i386 ELF documentation for details as to how to read the section header(s)). Note that
shdr_num may be 0, indicating no symbols, even if bit 5 in the flags word is set. */
#define MULTIBOOT_FLAGS_ELF_SECTIONS   0x00000020

/* bit 6 in the 'flags' word is set, then the 'mmap_*' fields are valid, and indicate the address
and length of a buffer containing a memory map of the machine provided by the BIOS. 'mmap_addr'
is the address, and 'mmap_length' is the total size of the buffer. The buffer consists of one or
more of the following size/structure pairs ('size' is really used for skipping to the next pair):

        +-------------------+
    -4  | size              |
        +-------------------+
    0   | base_addr         |
    8   | length            |
    16  | type              |
        +-------------------+

where 'size' is the size of the associated structure in bytes, which can be greater than the minimum
of 20 bytes. 'base_addr' is the starting address. 'length' is the size of the memory region in bytes.
'type' is the variety of address range represented, where a value of 1 indicates available RAM, value
of 3 indicates usable memory holding ACPI information, value of 4 indicates reserved memory which needs
to be preserved on hibernation, value of 5 indicates a memory which is occupied by defective RAM modules
and all other values currently indicated a reserved area. The map provided is guaranteed to list all
standard RAM that should be available for normal use. */
#define MULTIBOOT_FLAGS_MMAP           0x00000040

typedef struct multiboot_info_t {
    uint32_t flags;
    uint32_t mem_lower;              /* in kb. Lower memory starts at address 0 */
    uint32_t mem_upper;              /* in kb. Upper memory starts at address 1 megabyte */
    uint8_t boot_device[4];
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t sym_num;
    uint32_t sym_size;
    uint32_t sym_addr;
    uint32_t sym_shndx;
    uint32_t mmap_length;
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
} multiboot_info_t;

#define MULTIBOOT_MMAP_TYPE_NONE     0
#define MULTIBOOT_MMAP_TYPE_RAM      1
#define MULTIBOOT_MMAP_TYPE_RESERVED 2
#define MULTIBOOT_MMAP_TYPE_ACPI     3
#define MULTIBOOT_MMAP_TYPE_NVS      4
#define MULTIBOOT_MMAP_TYPE_BADRAM   5
#define MULTIBOOT_MMAP_TYPE_COUNT    6

typedef struct multiboot_mmap_t {
	uint32_t size;  /* Size of the entry excluding this field (normally 20) */
    uint32_t addr1; /* Physical start address */
    uint32_t addr2; /* Physical start address */
    uint32_t len1;  /* Length in bytes */
    uint32_t len2;  /* Length in bytes */
    uint32_t type;  /* 1 = available RAM */
} multiboot_mmap_t;

typedef struct kmmap_t kmmap_t;
void mb_init(kmmap_t* kmmap);

#endif
