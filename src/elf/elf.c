/* elf.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

 #include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <elf.h>
#include <paging.h>
#include <align.h>
#include <align.h>
#include <kdprint.h>

# define ELFMAG0	0x7F /* e_ident[EI_MAG0] */
# define ELFMAG1	'E'  /* e_ident[EI_MAG1] */
# define ELFMAG2	'L'  /* e_ident[EI_MAG2] */
# define ELFMAG3	'F'  /* e_ident[EI_MAG3] */

# define ELFDATA2LSB 1   /* Little Endian */
# define ELFCLASS32	 1   /* 32-bit Architecture */

#define ET_NONE    0x0000 /* No file type */
#define ET_REL     0x0001 /* Relocatable file */
#define ET_EXEC    0x0002 /* Executable file */
#define ET_DYN     0x0003 /* Shared object file */
#define ET_CORE    0x0004 /* Core file */
#define ET_LOPROC  0xFF00 /* Processor specific */
#define ET_HIPROC  0xFFFF /* Processor specific */

#define EM_NONE    0x0000
#define EM_M32     0x0001 /* AT&T WE 32100 */
#define EM_SPARC   0x0002 /* SPARC */
#define EM_386     0x0003 /* Intel 80386 */
#define EM_68K     0x0004 /* Motorola 68000 */
#define EM_88K     0x0005 /* Motorola 88000 */
#define EM_860     0x0007 /* Intel 80860 */
#define EM_MIPS    0x0008 /* MIPS RS3000 */

#define EI_MAG0    0x0000 /* File ID */
#define EI_MAG1    0x0001 /* File ID */
#define EI_MAG2    0x0002 /* File ID */
#define EI_MAG3    0x0003 /* File ID */
#define EI_CLASS   0x0004 /* File class */
#define EI_DATA    0x0005 /* Data encoding */
#define EI_VERSION 0x0006 /* File version */
#define EI_PAD     0x0007 /* Start of padding bytes */

#define SHN_UNDEF  0x00   /* Undefined/Not Present */
#define SHN_ABS    0xFFF1 /* Absolute symbol */

#define SHT_NULL	 0    /* Null section */
#define SHT_PROGBITS 1    /* Program information */
#define SHT_SYMTAB	 2    /* Symbol table */
#define SHT_STRTAB	 3    /* String table */
#define SHT_RELA	 4    /* Relocation (w/ addend) */
#define SHT_NOBITS	 8    /* Not present in file */
#define SHT_REL		 9    /* Relocation (no addend) */

#define SHF_WRITE    0x01 /* Writable section */
#define SHF_ALLOC    0x02 /* Exists in memory */

#define EV_NONE      0x00
#define EV_CURRENT   0x01

#define ELF32_ST_BIND(x) ((x) >> 4)
#define ELF32_ST_TYPE(x) ((x) & 0x0F)

#define STB_LOCAL  0 /* Local scope */
#define STB_GLOBAL 1 /* Global scope */
#define STB_WEAK   2 /* Weak, (ie. __attribute__((weak))) */

#define STT_NOTYPE 0 /* No type */
#define STT_OBJECT 1 /* Variables, arrays, etc */
#define STT_FUNC   2 /* Methods or functions */

#define ELF32_R_SYM(x) ((x) >> 8)
#define ELF32_R_TYPE(x) ((uint8_t)(x))

#define R_386_NONE 0 /* No relocation */
#define R_386_32   1 /* Symbol + Offset */
#define R_386_PC32 2 /* Symbol + Offset - Section Offset */

#define DO_386_32(s, a)      ((s) + (a))
#define DO_386_PC32(s, a, p) ((s) + (a) - (p))

#define sheader(x)   ((Elf32_Shdr*)((uintptr_t)(x) + (x)->e_shoff))
#define section(x,i) (&sheader(x)[(i)])

/* 
 * Elf32_Addr  -> 4 -> unsigned program address
 * Elf32_Half  -> 2 -> unsigned medium integer
 * Elf32_Off   -> 4 -> unsigned file offset
 * Elf32_Sword -> 4 -> signed large integer
 * Elf32_Word  -> 4 -> unsigned large integer
 */
 
 /* ELF Section Header */
typedef struct {
	uint32_t name;
	uint32_t type;
	uint32_t flags;
	uint32_t addr;
	uint32_t offset;
	uint32_t size;
	uint32_t link;
	uint32_t info;
	uint32_t addralign;
	uint32_t entsize;
} Elf32_Shdr;

 /* ELF Symbol Table */
typedef struct {
	uint32_t name;
	uint32_t value;
	uint32_t size;
	uint8_t info;
	uint8_t other;
	uint16_t shndx;
} Elf32_Sym;

typedef struct {
	uint32_t offset;
	uint32_t info;
} Elf32_Rel;

typedef struct {
	uint32_t offset;
	uint32_t info;
	int32_t addend;
} Elf32_Rela;

 /* ELF Program Header */
typedef struct {
	uint32_t type;
	uint32_t offset;
	uint32_t vaddr;
	uint32_t paddr;
	uint32_t filesz;
	uint32_t memsz;
	uint32_t flags;
	uint32_t align;
} Elf32_Phdr;

static void* load_rel(Elf32_Ehdr* ehdr);
static int check_supported(Elf32_Ehdr* ehdr);

void* elf_load_file(void* file) {
    if (file == NULL) {
        return NULL;
    }

	Elf32_Ehdr* ehdr = (Elf32_Ehdr*)file;

	if (!check_supported(ehdr)) {
		kdprint("[ELFLDR] ELF File cannot be loaded\n");
		return NULL;
	}

	switch(ehdr->e_type) {
		case ET_REL:
			return load_rel(ehdr);
		default:		
			kdprint("[ELFLDR] Unsupported ELF file type\n");
			return NULL;
	}
}

static int check_supported(Elf32_Ehdr* ehdr) {
	if (ehdr->e_ident[EI_MAG0] != ELFMAG0) {
		kdprint("[ELFLDR] ELF Header EI_MAG0 incorrect\n");
		return 0;
	}
	if (ehdr->e_ident[EI_MAG1] != ELFMAG1) {
		kdprint("[ELFLDR] ELF Header EI_MAG1 incorrect\n");
		return 0;
	}
	if (ehdr->e_ident[EI_MAG2] != ELFMAG2) {
		kdprint("[ELFLDR] ELF Header EI_MAG2 incorrect\n");
		return 0;
	}
	if (ehdr->e_ident[EI_MAG3] != ELFMAG3) {
		kdprint("[ELFLDR] ELF Header EI_MAG3 incorrect\n");
		return 0;
	}
	if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
		kdprint("[ELFLDR] Unsupported ELF File Class\n");
		return 0;
	}
	if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
		kdprint("[ELFLDR] Unsupported ELF File byte order\n");
		return 0;
	}
	if (ehdr->e_machine != EM_386) {
		kdprint("[ELFLDR] Unsupported ELF File target\n");
		return 0;
	}
	if (ehdr->e_ident[EI_VERSION] != EV_CURRENT) {
		kdprint("[ELFLDR] Unsupported ELF File version\n");
		return 0;
	}
	if (ehdr->e_type != ET_REL && ehdr->e_type != ET_EXEC) {
		kdprint("[ELFLDR] Unsupported ELF File type\n");
		return 0;
	}
	return 1;
}
static uintptr_t get_symval(Elf32_Ehdr* ehdr, size_t table, size_t i) {
	if (table == SHN_UNDEF || i == SHN_UNDEF) {
        return 0;
    }

	Elf32_Shdr* symtab = section(ehdr, table);
	uint32_t symtab_entries = symtab->size / symtab->entsize;
	
    if (i >= symtab_entries) {
		kdprint("[ELFLDR] Symbol Index out of Range (%d:%u).\n", table, i);
		return 0;
	}

	uintptr_t symaddr = (uintptr_t)ehdr + symtab->offset;
	Elf32_Sym* symbol = &((Elf32_Sym*)symaddr)[i];

    if (symbol->shndx == SHN_UNDEF) {
		/* External symbol, lookup value */
		Elf32_Shdr* strtab = section(ehdr, symtab->link);
		const char* name = (const char*)ehdr + strtab->offset + symbol->name;

		extern void* lookup_symbol(const char* name);
		void* target = lookup_symbol(name);

		if (target == NULL) {
			// Extern symbol not found */
			if (ELF32_ST_BIND(symbol->info) & STB_WEAK) {
				// Weak symbol initialized as 0 */
				return 0;
			}
            else {
				kdprint("[ELFLDR] Undefined External Symbol : %s.\n", name);
				return 0;
			}
		} 
        else {
			return (uintptr_t)target;
		}
    }
	else if (symbol->shndx == SHN_ABS) {
		// Absolute symbol */
		return symbol->value;
	}
    else {
		// Internally defined symbol */
		Elf32_Shdr* target = section(ehdr, symbol->shndx);
		return (uintptr_t)ehdr + symbol->value + target->offset;
	}
}
static int do_reloc(Elf32_Ehdr* ehdr, Elf32_Rel* rel, Elf32_Shdr* reltab) {
	Elf32_Shdr* target = section(ehdr, reltab->info);

	uintptr_t addr = (uintptr_t)ehdr + target->offset;
	int* ref = (int*)(addr + rel->offset);

    /* Symbol value */
	int symval = 0;
	if (ELF32_R_SYM(rel->info) != SHN_UNDEF) {
		symval = get_symval(ehdr, reltab->link, ELF32_R_SYM(rel->info));
		if (symval == 0) {
            return 0;
        }
	}

    /* Relocate based on type */
	switch (ELF32_R_TYPE(rel->info)) {
		case R_386_NONE:
			/* No relocation */
			break;
		case R_386_32:
			/* Symbol + Offset */
			*ref = DO_386_32(symval, *ref);
			break;
		case R_386_PC32:
			/* Symbol + Offset - Section Offset */
			*ref = DO_386_PC32(symval, *ref, (int)ref);
			break;
		default:
			/* Relocation type not supported, display kdprint and return */
			kdprint("[ELFLDR] Unsupported Relocation Type (%d).\n", ELF32_R_TYPE(rel->info));
			return 0;
	}
	return symval;
}
static void* load_rel(Elf32_Ehdr* ehdr) {
    Elf32_Shdr* shdr = sheader(ehdr);
	
	/* Iterate over section headers */
	for (uint16_t i = 0; i < ehdr->e_shnum; i++) {
		Elf32_Shdr* section = &shdr[i];
		
        switch (section->type) {
            case SHT_REL: {
                for (uint32_t idx = 0; idx < section->size / section->entsize; idx++) {
                    Elf32_Rel* reltab = &((Elf32_Rel*)((uintptr_t)ehdr + section->offset))[idx];
                    if (!do_reloc(ehdr, reltab, section)) {
                        kdprint("[ELFLDR] Failed to relocate symbol\n");
                        return 0;
                    }
			    }
            } break;

            case SHT_NOBITS: {
                if (!section->size) {
                    continue;
                }

                if (section->flags & SHF_ALLOC) {
                    void* mem = malloc(section->size);
                    memset(mem, 0, section->size);

                    section->offset = (uintptr_t)mem - (uintptr_t)ehdr;
                    kdprint("[ELFLDR] Allocated memory for a section (%ld)\n", section->size);
                }
            } break;
        }
	}

    kdprint("[ELFLDR] Loaded ELF -> entry=0x%08X\n", ehdr->e_entry);
	return (void*)ehdr->e_entry;
}
