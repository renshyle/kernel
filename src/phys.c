#include <limine.h>
#include <stdbool.h>
#include <stdlib.h>

#include "debug.h"
#include "panic.h"
#include "phys.h"
#include "virt.h"

struct {
    uint64_t idx;
    uint64_t *stack;
} phys_mem_stack;

void phys_init(struct limine_memmap_response *memmap_response)
{
    uint64_t free_memory = 0;

    debug_write_string("memory map:\n");
    // calculate amount of free (usable) memory and print memory map
    for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_response->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            // limine aligns base and length for usable
            free_memory += entry->length;
        }

        debug_write_string("  0x");
        debug_write_uint64(entry->base);
        debug_write_string(" -> 0x");
        debug_write_uint64(entry->base + entry->length);
        debug_write_string(": ");

        switch (entry->type) {
            case LIMINE_MEMMAP_USABLE:
                debug_write_string("usable");
                break;
            case LIMINE_MEMMAP_RESERVED:
                debug_write_string("reserved");
                break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                debug_write_string("acpi reclaimable");
                break;
            case LIMINE_MEMMAP_ACPI_NVS:
                debug_write_string("acpi nvs");
                break;
            case LIMINE_MEMMAP_BAD_MEMORY:
                debug_write_string("bad memory");
                break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                debug_write_string("bootloader reclaimable");
                break;
            case LIMINE_MEMMAP_KERNEL_AND_MODULES:
                debug_write_string("kernel and modules");
                break;
            case LIMINE_MEMMAP_FRAMEBUFFER:
                debug_write_string("framebuffer");
                break;
            default:
                debug_write_string("unknown");
                break;
        }

        debug_write_string("\n");
    }

    uint64_t stack_req_size = free_memory / PAGE_SIZE * sizeof(uint64_t*);

    // find suitable place for stack
    for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_response->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            if (entry->length >= stack_req_size) {
                // limine identity maps usable pages so this is fine
                phys_mem_stack.stack = (uint64_t*) entry->base;
                break;
            }
        }
    }

    if (phys_mem_stack.stack == NULL) {
        // no memory for stack
        panic("could not allocate memory for physical memory stack");
    }

    phys_mem_stack.idx = 0;

    // mark usable pages as free
    bool found_stack = false;
    for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_response->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            if (!found_stack) {
                if (entry->length >= stack_req_size) {
                    // the stack we allocated is in this area
                    uint64_t aligned_stack_size = ((stack_req_size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
                    uint64_t new_base = entry->base + aligned_stack_size;
                    uint64_t new_length = entry->length - aligned_stack_size;

                    if (new_length > 0) {
                        for (uint64_t j = 0; j < new_length; j += PAGE_SIZE) {
                            phys_free_page(new_base + j);
                        }
                    }

                    found_stack = true;
                    continue;
                }
            }

            for (uint64_t j = 0; j < entry->length; j += PAGE_SIZE) {
                phys_free_page(entry->base + j);
            }
        }
    }

    debug_write_string("free memory (bytes): 0x");
    debug_write_uint64(free_memory - ((stack_req_size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE);
    debug_write_string("\nfree pages (bytes):  0x");
    debug_write_uint64(phys_mem_stack.idx * PAGE_SIZE);
    debug_write_string("\nthe two values should be equal\n");
}

void phys_free_page(uint64_t page)
{
    phys_mem_stack.stack[phys_mem_stack.idx++] = page;
}

uint64_t phys_alloc_page(void)
{
    if (phys_mem_stack.idx == 0) {
        // no memory
        return 0;
    }

    return phys_mem_stack.stack[--phys_mem_stack.idx];
}

// only for virt_init
void phys_virt_reset_mem_stack_address(void)
{
    phys_mem_stack.stack = PHYSICAL_TO_VIRTUAL(phys_mem_stack.stack);
}
