#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "debug.h"
#include "panic.h"
#include "phys.h"
#include "virt.h"

#define PAGE_PRESENT 0x01
#define PAGE_WRITABLE 0x02
#define PAGE_USER_ACCESS 0x04
#define PAGE_HUGE 0x80
// PAGE_ALLOCATED means that the page can be free using phys_free_page()
// 0x200 is a free bit in the paging structures
#define PAGE_ALLOCATED 0x200
#define PAGE_ADDRESS_MASK 0x000fffffffff000

#define PAGES_LVL4 134217728
#define PAGES_LVL3 262144
#define PAGES_LVL2 512
#define PAGES_LVL1 1

pml4e kernel_pml4[512] __attribute__((aligned(4096)));
pdpe kernel_pdp1[512] __attribute__((aligned(4096)));
pdpe kernel_pdp_phys[512] __attribute__((aligned(4096)));

// KERNEL_VIRTUAL_START
pde kernel_pd1[512] __attribute__((aligned(4096)));

// 0xffffffffc0000000
pde kernel_pd2[512] __attribute__((aligned(4096)));

uint64_t kernel_physical_address;

void virt_init(uint64_t kernel_phys_address)
{
    if ((kernel_phys_address & 0xfff) != 0) {
        panic("kernel physical address not page-aligned");
    }

    kernel_physical_address = kernel_phys_address;

    debug_write_string("creating kernel memory mappings\n");

    // map 2 gb starting at kernel_phys_address to KERNEL_VIRTUAL_START (0xffffffff80000000)

    kernel_pml4[511] = ((uint64_t) &kernel_pdp1 - KERNEL_VIRTUAL_START + kernel_phys_address) | PAGE_PRESENT | PAGE_WRITABLE;

    kernel_pdp1[510] = ((uint64_t) &kernel_pd1 - KERNEL_VIRTUAL_START + kernel_phys_address) | PAGE_PRESENT | PAGE_WRITABLE;
    kernel_pdp1[511] = ((uint64_t) &kernel_pd2 - KERNEL_VIRTUAL_START + kernel_phys_address) | PAGE_PRESENT | PAGE_WRITABLE;

    for (int i = 0; i < 512; i++) {
        uint64_t page1 = phys_alloc_page();
        uint64_t page2 = phys_alloc_page();

        if (page1 == 0 || page2 == 0) {
            panic("out of memory");
        }

        kernel_pd1[i] = page1 | PAGE_PRESENT | PAGE_WRITABLE;
        kernel_pd2[i] = page2 | PAGE_PRESENT | PAGE_WRITABLE;
        pte *page_table1 = (pte*) page1;
        pte *page_table2 = (pte*) page2;

        for (int j = 0; j < 512; j++) {
            page_table1[j] = (kernel_phys_address + i * 0x200000 + j * 0x1000) | PAGE_PRESENT | PAGE_WRITABLE;
            page_table2[j] = (kernel_phys_address + 0x40000000 + i * 0x200000 + j * 0x1000) | PAGE_PRESENT | PAGE_WRITABLE;
        }
    }

    // map first 512 gb of physical memory to PHYSICAL_MEMORY (0xffffff0000000000)

    kernel_pml4[510] = ((uint64_t) &kernel_pdp_phys - KERNEL_VIRTUAL_START + kernel_phys_address) | PAGE_PRESENT | PAGE_WRITABLE;

    for (uint64_t i = 0; i < 10; i++) {
        uint64_t page = phys_alloc_page();

        if (page == 0) {
            panic("out of memory");
        }

        kernel_pdp_phys[i] = page | PAGE_PRESENT | PAGE_WRITABLE;
        pde *page_dir = (pde*) page;

        for (uint64_t j = 0; j < 512; j++) {
            page_dir[j] = (i * 0x40000000 + j * 0x200000) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_HUGE;
        }
    }

    debug_write_string("created kernel memory mappings\n");

    set_cr3((uint64_t) &kernel_pml4 - KERNEL_VIRTUAL_START + kernel_phys_address);

    phys_virt_reset_mem_stack_address();
}

pml4e *virt_create_page_map(void)
{
    uint64_t page = phys_alloc_page();

    if (page == 0) {
        return NULL;
    }

    pml4e *pml4 = PHYSICAL_TO_VIRTUAL(page);

    memcpy(pml4, kernel_pml4, sizeof(kernel_pml4));

    return pml4;
}

// only should be called for page maps allocated using virt_create_page_map()
void virt_destroy_page_map(pml4e *page_map)
{
    // reset the current page map to kernel_pml4 if the current page map is the one being destroyed
    uint64_t cr3 = get_cr3();
    if (cr3 == kvirtual_to_physical((uint64_t) page_map)) {
        set_cr3(kvirtual_to_physical((uint64_t) kernel_pml4));
    }

    for (int i = 0; i < (int) (MAX_USERSPACE_ADDRESS / 512 / 1024 / 1024 / 1024); i++) {
        if (page_map[i] & PAGE_PRESENT) {
            pdpe *pdp = PHYSICAL_TO_VIRTUAL(page_map[i] & PAGE_ADDRESS_MASK);

            for (int j = 0; j < 512; j++) {
                if (pdp[j] & PAGE_PRESENT) {
                    pde *pd = PHYSICAL_TO_VIRTUAL(pdp[j] & PAGE_ADDRESS_MASK);

                    for (int k = 0; k < 512; k++) {
                        if (pd[k] & PAGE_PRESENT) {
                            pte *pt = PHYSICAL_TO_VIRTUAL(pd[k] & PAGE_ADDRESS_MASK);

                            for (int l = 0; l < 512; l++) {
                                if (pt[l] & PAGE_PRESENT && pt[l] & PAGE_ALLOCATED) {
                                    phys_free_page(pt[l] & PAGE_ADDRESS_MASK);
                                }
                            }

                            if (pd[k] & PAGE_ALLOCATED) {
                                phys_free_page(pd[k] & PAGE_ADDRESS_MASK);
                            }
                        }
                    }

                    if (pdp[j] & PAGE_ALLOCATED) {
                        phys_free_page(pdp[j] & PAGE_ADDRESS_MASK);
                    }
                }
            }

            if (page_map[i] & PAGE_ALLOCATED) {
                phys_free_page(page_map[i] & PAGE_ADDRESS_MASK);
            }
        }
    }

    phys_free_page(kvirtual_to_physical((uint64_t) page_map));
}

uint64_t virt_find_free_area(pml4e *page_map, uint64_t pages, uint64_t flags)
{
    bool user = flags & VIRT_FLAG_USER;
    uint64_t search_start = user ? 0x1000 : 0xffffff8000000000;
    uint64_t search_end = user ? 0x7ffffffff000 : 0xfffffffffffff000;

    uint64_t found_pages = 0;
    for (uint64_t i = search_start; i < search_end; i += PAGE_SIZE) {
        if (virt_is_page_free(page_map, i)) {
            found_pages++;

            if (found_pages >= pages) {
                return i - (found_pages - 1) * PAGE_SIZE;
            }
        } else {
            found_pages = 0;
        }
    }

    return 0;
}

bool virt_is_page_free(pml4e *page_map, uint64_t address)
{
    int pml4i = (address >> 39) & 0x1ff;
    int pdpi = (address >> 30) & 0x1ff;
    int pdi = (address >> 21) & 0x1ff;
    int pti = (address >> 12) & 0x1ff;

    if ((page_map[pml4i] & PAGE_PRESENT) == 0) {
        return true;
    }

    pdpe *pdp = PHYSICAL_TO_VIRTUAL(page_map[pml4i] & PAGE_ADDRESS_MASK);
    if ((pdp[pdpi] & PAGE_PRESENT) == 0) {
        return true;
    }

    pde *pd = PHYSICAL_TO_VIRTUAL(pdp[pdpi] & PAGE_ADDRESS_MASK);
    if ((pd[pdi] & PAGE_PRESENT) == 0) {
        return true;
    }

    pte *pt = PHYSICAL_TO_VIRTUAL(pd[pdi] & PAGE_ADDRESS_MASK);
    if ((pt[pti] & PAGE_PRESENT) == 0) {
        return true;
    }

    return false;
}

void virt_map(pml4e *page_map, uint64_t virt_address, uint64_t phys_address, uint64_t flags)
{
    bool user = flags & VIRT_FLAG_USER;
    int pml4i = (virt_address >> 39) & 0x1ff;
    int pdpi = (virt_address >> 30) & 0x1ff;
    int pdi = (virt_address >> 21) & 0x1ff;
    int pti = (virt_address >> 12) & 0x1ff;

    if ((page_map[pml4i] & PAGE_PRESENT) == 0) {
        uint64_t page = phys_alloc_page();

        if (page == 0) {
            panic("out of memory");
        }

        memset(PHYSICAL_TO_VIRTUAL(page), 0, PAGE_SIZE);
        page_map[pml4i] = page | PAGE_PRESENT | PAGE_WRITABLE | (user ? PAGE_USER_ACCESS : 0) | PAGE_ALLOCATED;
    }

    pdpe *pdp = PHYSICAL_TO_VIRTUAL(page_map[pml4i] & PAGE_ADDRESS_MASK);
    if ((pdp[pdpi] & PAGE_PRESENT) == 0) {
        uint64_t page = phys_alloc_page();

        if (page == 0) {
            panic("out of memory");
        }

        memset(PHYSICAL_TO_VIRTUAL(page), 0, PAGE_SIZE);
        pdp[pdpi] = page | PAGE_PRESENT | PAGE_WRITABLE | (user ? PAGE_USER_ACCESS : 0) | PAGE_ALLOCATED;
    }

    pde *pd = PHYSICAL_TO_VIRTUAL(pdp[pdpi] & PAGE_ADDRESS_MASK);
    if ((pd[pdi] & PAGE_PRESENT) == 0) {
        uint64_t page = phys_alloc_page();

        if (page == 0) {
            panic("out of memory");
        }

        memset(PHYSICAL_TO_VIRTUAL(page), 0, PAGE_SIZE);
        pd[pdi] = page | PAGE_PRESENT | PAGE_WRITABLE | (user ? PAGE_USER_ACCESS : 0) | PAGE_ALLOCATED;
    }

    pte *pt = PHYSICAL_TO_VIRTUAL(pd[pdi] & PAGE_ADDRESS_MASK);
    pt[pti] = phys_address | PAGE_PRESENT | PAGE_WRITABLE | (user ? PAGE_USER_ACCESS : 0) | ((flags & VIRT_FLAG_ALLOC) ? PAGE_ALLOCATED : 0);
}

void virt_unmap(pml4e *page_map, uint64_t virt_address)
{
    int pml4i = (virt_address >> 39) & 0x1ff;
    int pdpi = (virt_address >> 30) & 0x1ff;
    int pdi = (virt_address >> 21) & 0x1ff;
    int pti = (virt_address >> 12) & 0x1ff;

    if ((page_map[pml4i] & PAGE_PRESENT) == 0) {
        return;
    }

    pdpe *pdp = PHYSICAL_TO_VIRTUAL(page_map[pml4i] & PAGE_ADDRESS_MASK);
    if ((pdp[pdpi] & PAGE_PRESENT) == 0) {
        return;
    }

    pde *pd = PHYSICAL_TO_VIRTUAL(pdp[pdpi] & PAGE_ADDRESS_MASK);
    if ((pd[pdi] & PAGE_PRESENT) == 0) {
        return;
    }

    pte *pt = PHYSICAL_TO_VIRTUAL(pd[pdi] & PAGE_ADDRESS_MASK);

    if (pt[pti] & PAGE_PRESENT && pt[pti] & PAGE_ALLOCATED) {
        phys_free_page(pt[pti] & PAGE_ADDRESS_MASK);
    }

    pt[pti] = 0;
    invlpg(virt_address);
}

uint64_t kvirtual_to_physical(uint64_t address)
{
    if (address < PHYSICAL_MEMORY) {
        // PHYSICAL_MEMORY is the lowest address mapped in kernel memory
        return 0;
    }

    if (address < KERNEL_VIRTUAL_START) {
        return address - PHYSICAL_MEMORY;
    } else {
        return address - KERNEL_VIRTUAL_START + kernel_physical_address;
    }
}
