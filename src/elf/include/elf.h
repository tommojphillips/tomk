/* elf.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _ELF_H
#define _ELF_H

#include <stdint.h>
#include <stddef.h>

#define EI_NIDENT  0x0010 /* sizeof e_ident[] */

 /* ELF Header */
typedef struct {
    uint8_t e_ident[EI_NIDENT];
    uint16_t e_type;                 /* Object file type */
    uint16_t e_machine;              /* Target architecture */
    uint32_t e_version;              /* Object file version */
    uint32_t e_entry;
    uint32_t e_phoff;                /* Program header offset */
    uint32_t e_shoff;                /* Section header offset */
    uint32_t e_flags;                /* Processor-specific flags */
    uint16_t e_ehsize;               /* ELF headers size */
    uint16_t e_phentsize;            /* Program header entry size */
    uint16_t e_phnum;                /* Program header entry count */
    uint16_t e_shentsize;            /* Section header entry size */
    uint16_t e_shnum;                /* Section header entry count */
    uint16_t e_shstrndx;             /* Section header table index */
} Elf32_Ehdr;

int elf_loader(uint8_t* data, size_t size);

#endif
