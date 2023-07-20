#include <stdint.h>

#include "gdt.h"

#define GDT_ATTR_PRESENT 0b10000000
#define GDT_ATTR_DPL0 0
#define GDT_ATTR_DPL3 0b01100000
#define GDT_ATTR_NORMAL 0b00010000
#define GDT_ATTR_EXECUTABLE 0b00001000
#define GDT_ATTR_CODE_READABLE 0b00000010
#define GDT_ATTR_DATA_WRITABLE 0b00000010

#define GDT_FLAG_LONG_MODE 0b0010

struct __attribute__((__packed__)) {
    uint16_t size;
    uint64_t offset;
} gdtr;

struct gdt_descriptor gdt_descriptors[5];

void gdt_init(void)
{
    gdt_set_descriptor(0, 0, 0, 0, 0); // null descriptor
    gdt_set_descriptor(1, 0x00000000, 0xfffff, GDT_ATTR_PRESENT | GDT_ATTR_DPL0 | GDT_ATTR_NORMAL | GDT_ATTR_EXECUTABLE | GDT_ATTR_CODE_READABLE, GDT_FLAG_LONG_MODE); // 64-bit code
    gdt_set_descriptor(2, 0x00000000, 0xfffff, GDT_ATTR_PRESENT | GDT_ATTR_DPL0 | GDT_ATTR_NORMAL | GDT_ATTR_DATA_WRITABLE, GDT_FLAG_LONG_MODE); // 64-bit data
    gdt_set_descriptor(3, 0x00000000, 0xfffff, GDT_ATTR_PRESENT | GDT_ATTR_DPL3 | GDT_ATTR_NORMAL | GDT_ATTR_EXECUTABLE | GDT_ATTR_CODE_READABLE, GDT_FLAG_LONG_MODE); // 64-bit ring 3code
    gdt_set_descriptor(4, 0x00000000, 0xfffff, GDT_ATTR_PRESENT | GDT_ATTR_DPL3 | GDT_ATTR_NORMAL | GDT_ATTR_DATA_WRITABLE, GDT_FLAG_LONG_MODE); // 64-bit ring 3 data

    gdtr.offset = (uint64_t) &gdt_descriptors;
    gdtr.size = 5 * sizeof(struct gdt_descriptor) - 1;

    gdt_load();
}

void gdt_set_descriptor(int i, uint32_t base, uint32_t limit, uint8_t attr, uint8_t flags)
{
    gdt_descriptors[i].base_low = base & 0xffff;
    gdt_descriptors[i].base_mid = (base >> 16) & 0xff;
    gdt_descriptors[i].base_high = (base >> 24) & 0xff;
    gdt_descriptors[i].limit_low = limit & 0xffff;
    gdt_descriptors[i].limit_high = (limit >> 16) & 0xf;
    gdt_descriptors[i].attr = attr;
    gdt_descriptors[i].flags = flags;
}
