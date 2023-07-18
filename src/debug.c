#include <stdint.h>

#include "io.h"

#define COM1 0x3f8

void debug_init(void)
{
#ifdef DEBUG
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
    outb(COM1 + 4, 0x1E);
    outb(COM1 + 0, 0xAE);

    // Assume serial works
    outb(COM1 + 4, 0x0f);
#endif
}

void debug_write_byte(char c)
{
#ifdef DEBUG
    while ((inb(COM1 + 5) & 0x20) == 0);

    outb(COM1, c);
#endif
}

void debug_write_string(char *str)
{
#ifdef DEBUG
    char c;

    while ((c = *(str++)) != 0) {
        debug_write_byte(c);
    }
#endif
}

void debug_write_uint32(uint32_t val)
{
#ifdef DEBUG
    for (int i = 0; i < 32; i += 4) {
        debug_write_byte("0123456789abcdef"[(val >> (28 - i)) & 0x0f]);
    }
#endif
}

void debug_write_uint64(uint64_t val)
{
#ifdef DEBUG
    for (int i = 0; i < 64; i += 4) {
        debug_write_byte("0123456789abcdef"[(val >> (60 - i)) & 0x0f]);
    }
#endif
}
