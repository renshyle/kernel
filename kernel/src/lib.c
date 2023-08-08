#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "phys.h"
#include "virt.h"

void *memcpy(void *restrict s1, const void *restrict s2, size_t n)
{
    char *s1c = s1;
    const char *s2c = s2;

    for (size_t i = 0; i < n; i++) {
        s1c[i] = s2c[i];
    }

    return s1;
}

void *memset(void *s, int c, size_t n)
{
    char *s1 = s;

    for (size_t i = 0; i < n; i++) {
        s1[i] = (unsigned char) c;
    }

    return s;
}

void *libnalloc_alloc(size_t pages)
{
    uint64_t area = virt_find_free_area(kernel_pml4, pages, 0);

    if (area == 0) {
        return NULL;
    }

    for (size_t i = 0; i < pages; i++) {
        uint64_t page = phys_alloc_page();

        if (page == 0) {
            return NULL;
        }

        virt_map(kernel_pml4, area + i * PAGE_SIZE, page, VIRT_FLAG_ALLOC);
    }

    return (void*) area;
}

void libnalloc_free(void *ptr, size_t pages)
{
    for (size_t i = 0; i < pages; i++) {
        virt_unmap(kernel_pml4, (uint64_t) ptr + i * PAGE_SIZE);
    }
}

void libnalloc_lock(void)
{
    // i'm pretty sure this isn't needed yet
}

void libnalloc_unlock(void)
{

}
