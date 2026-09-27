/* fat32.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef _FAT32_H
#define _FAT32_H

#include <stdint.h>
#include <stddef.h>

typedef struct bpb32_t {
    uint8_t jmp_boot[3];            /* BS  - Jump instruction to boot code */
    char oem_name[8];               /* BS  - OEM name */
    uint16_t bps;                   /* BPB - Bytes per sector */
    uint8_t spc;                    /* BPB - Sectors per cluster */
    uint16_t num_rev_sector;        /* BPB - Number of reserved sectors in the Reserved region of the volume */
    uint8_t num_fat;                /* BPB - The count of FAT data structures on the volume */
    uint16_t root_ent_count;        /* BPB  */
    uint16_t total_sectors16;       /* BPB - The old 16-bit total count of sectors on the volume */
    uint8_t media;                  /* BPB */
    uint16_t fatsz16;               /* BPB */
    uint16_t spt;                   /* BPB - Sectors per track */
    uint16_t num_heads;             /* BPB */
    uint32_t hidden_sectors;        /* BPB */
    uint32_t total_sectors32;       /* BPB - The new 32-bit total count of sectors on the volume */
    uint32_t fatsz32;               /* BPB */
    uint16_t ext_flags;             /* BPB */
    uint16_t fs_version;            /* BPB */
    uint32_t root_cluster;          /* BPB */
    uint16_t fs_info;               /* BPB */
    uint16_t bk_boot_sec;           /* BPB */
    uint8_t reserved[12];           /* BPB */
} bpb32_t;

#endif
