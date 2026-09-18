/* kspacedef.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KSPACEDEF_H
#define KSPACEDEF_H

/* Kernel Base Virtual Address (3GB) */
#define KVIRT     0xC0000000

/* Kernel End Virtual Address (exclusive) */
#define KVIRT_END 0xFFC00000

/* Kernel Physical Address (1MB) */
#define KPHYS     0x00100000

/* Convert kernel virtual address to physical address */
#define V2P(_virt_addr) ((_virt_addr) - KVIRT)

/* Convert physical address to kernel virtual address */
#define P2V(_phys_addr) (KVIRT + (_phys_addr))

#endif
