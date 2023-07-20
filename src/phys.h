#ifndef PHYS_H
#define PHYS_H

#include <limine.h>

#define PAGE_SIZE 4096

void phys_init(struct limine_memmap_response *memmap_response);

void phys_free_page(uint64_t page);
uint64_t phys_alloc_page(void);

#endif
