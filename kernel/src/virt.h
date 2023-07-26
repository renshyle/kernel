#ifndef VIRT_H
#define VIRT_H

#include <stdbool.h>
#include <stdint.h>

#define PHYSICAL_MEMORY 0xffffff0000000000
#define KERNEL_VIRTUAL_START 0xffffffff80000000
#define PHYSICAL_TO_VIRTUAL(x) ((void*) ((uint64_t) (x) + PHYSICAL_MEMORY))
#define MAX_USERSPACE_ADDRESS 0x7fffffffffff

#define VIRT_FLAG_USER 0x01
#define VIRT_FLAG_ALLOC 0x02

// 512 gb
typedef uint64_t pml4e;
// 1 gb
typedef uint64_t pdpe;
// 2 mb
typedef uint64_t pde;
// 4 kb
typedef uint64_t pte;

void virt_init(uint64_t kernel_phys_address);

pml4e *virt_create_page_map(void);
void virt_destroy_page_map(pml4e *page_map);
void virt_map(pml4e *page_map, uint64_t virt_address, uint64_t phys_address, uint64_t flags);
uint64_t virt_find_free_area(pml4e *page_map, uint64_t pages, uint64_t flags);
bool virt_is_page_free(pml4e *page_map, uint64_t address);
uint64_t kvirtual_to_physical(uint64_t address);

void set_cr3(uint64_t cr3);
uint64_t get_cr3(void);

#endif
