/* kspacedef.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef KSPACEDEF_H
#define KSPACEDEF_H

/* Kernel Virtual Address */
#define KVIRT 0xC0000000

/* Kernel Virtual Address */
#define KVIRT_END 0xFFC00000

/* Kernel Physical Address */
#define KPHYS 0x00100000

/* User Virtual Address Base */
#define UVIRT_BASE 0x00001000

/* User Virtual Address End */
#define UVIRT_END KVIRT

/* Convert kernel virtual address to physical address */
#define V2P(_virt_addr) ((_virt_addr) - KVIRT)

/* Convert physical address to kernel virtual address */
#define P2V(_phys_addr) (KVIRT + (_phys_addr))

/* Convert virtual address to PD index */
#define PD_IDX(_virt_addr) ((_virt_addr >> 22) & 0x3FF)

/* Convert virtual address to PT index */
#define PT_IDX(_virt_addr) ((_virt_addr >> 12) & 0x3FF)

#endif
