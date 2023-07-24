#ifndef GDT_H
#define GDT_H

struct __attribute__((__packed__)) gdt_descriptor {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t attr;
    uint8_t limit_high : 4;
    uint8_t flags : 4;
    uint8_t base_high;
};

void gdt_init(void);

void gdt_set_descriptor(int i, uint32_t base, uint32_t limit, uint8_t attr, uint8_t flags);

void gdt_load(void);

#endif
