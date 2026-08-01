/* src/kernel/paging.c */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define TO_PAGE(x) ((x) >> 12)
#define TO_INDEX(x) ((x) >> 22)

#define PAGE_SIZE 0x1000
#define PD_BASE   0x1000
#define PT_BASE   0x2000

#define PAGE_RO      0x000
#define PAGE_PRESENT 0x001
#define PAGE_RW      0x002
#define PAGE_SUPER   0x000
#define PAGE_USER    0x004

#define KBASE    0x020000
#define KLINK    0x020000
#define KDATA    0x020000
#define KDEV     0x020000

#define PAGE_PRESENT 0x001

/* Page Table Entry struct */
typedef struct pte_t {
    union {
        uint32_t dword;
        struct {
            uint32_t present    : 1; /* Present */
            uint32_t rw         : 1; /* 0 = read only; 1 = read and write */
            uint32_t us         : 1; /* 0 = supervisor; 1 = user */
            uint32_t r3         : 1; 
            uint32_t r4         : 1;
            uint32_t accessed   : 1; /* Accessed */
            uint32_t dirty      : 1; /* 0 = page is unmodified; 1 = page is dirty (only valid in PTE) */
            uint32_t r7         : 1;
            uint32_t r8         : 1;
            uint32_t available  : 3; /* Available for systems programmer use */
            uint32_t page_frame : 20;
        };
    };
} pte_t;

/* Page Directory Entry struct */
typedef pte_t pde_t;

pde_t* pg_locate_pde(uint32_t laddr);
pde_t* build_pde(uint32_t laddr, uint32_t flags);
pte_t* pg_locate_pte(uint32_t laddr);
pte_t* build_pte(uint32_t laddr, uint32_t paddr, uint32_t flags);
int pg_map_page(uint32_t laddr, uint32_t paddr, uint32_t flags);
int pg_map_pages(uint32_t laddr, uint32_t paddr, uint32_t flags, uint32_t count);

void pg_init(void) {
   memset((void*)PD_BASE, 0, 0x1000);

   for (size_t i = 0; i < sizeof(kmap) / sizeof(kmap[0]); ++i) {
        pg_map_pages(kmap[i].laddr, kmap[i].paddr, kmap[i].flags, TO_PAGE(kmap[i].paddr_end - kmap[i].paddr));
   }
}
pde_t* pg_locate_pde(uint32_t laddr) {
    uint32_t pd_index = TO_INDEX(laddr);
    uint32_t pde_addr = PD_BASE + (pd_index << 2);
    return (pde_t*)pde_addr;
}
pde_t* build_pde(uint32_t laddr, uint32_t flags) {
    pde_t* pde = pg_locate_pde(laddr);
    uint32_t pd_index = TO_INDEX(laddr);
    uint32_t pt_addr = PT_BASE + (pd_index * PAGE_SIZE);
    pde->dword = (pt_addr & 0xFFFFF000) | (flags & 0x00000FFF)| PAGE_PRESENT;
    return pde;
}

pte_t* pg_locate_pte(uint32_t laddr) {
    pte_t* pt = NULL;
    uint32_t pt_i = 0;
    uint32_t pd_index = 0;
    uint32_t pt_addr = 0;
    
    pd_index = TO_INDEX(laddr);
    pt_addr = PT_BASE + (pd_index * PAGE_SIZE);
    pt_i = TO_PAGE(laddr) & 0x3FF;
    pt = (pte_t*)(pt_addr);
    return &pt[pt_i];
}
pte_t* build_pte(uint32_t laddr, uint32_t paddr, uint32_t flags) {
    pte_t* pde = pg_locate_pde(laddr);
    if (!pde->present) {
        return NULL;
    }

    pte_t* pte = pg_locate_pte(laddr);        
    pte->dword = (paddr & 0xFFFFF000) | (flags & 0x00000FFF) | PAGE_PRESENT;
    return pte;
}

int pg_map_page(uint32_t laddr, uint32_t paddr, uint32_t flags) {
    /* Map page in page table
     laddr: linear address
     paddr: physical address
     flags: page permissions
    */

    if (!build_pde(laddr, flags)) {
        return 0;
    }
    if (!build_pte(laddr, paddr, flags)) {
        return 0;
    }

    return 1;
}

int pg_map_pages(uint32_t laddr, uint32_t paddr, uint32_t flags, uint32_t count) {
    /* Map pages
     laddr: linear address
     paddr: physical address
     flags: page permissions
     count: pages to map
    */

    while (count--) {
        if (!pg_map_page(laddr, paddr, flags)) {
            return 0;
        }
        laddr += PAGE_SIZE;
        paddr += PAGE_SIZE;
    }
    return 1;
}

