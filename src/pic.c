#include <stdbool.h>
#include <stdint.h>

#include "io.h"
#include "pic.h"

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define PIC_EOI 0x20
#define PIC_READ_ISR 0x0b

#define ICW4_8086 0x01

void pic_init(void)
{
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, 32); // map pic1 to 32-39
    io_wait();
    outb(PIC2_DATA, 40); // map pic2 to 40-47
    io_wait();
    outb(PIC1_DATA, 4);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC1_DATA, ICW4_8086);
    io_wait();

    // set ocw3 modes to isr read
    outb(PIC1_COMMAND, PIC_READ_ISR);
    outb(PIC2_COMMAND, PIC_READ_ISR);

    pic_set_irq_mask(PIC1, 0x00);
    pic_set_irq_mask(PIC2, 0x00);
}

uint8_t pic_get_irq_mask(uint16_t pic)
{
    return inb(pic);
}

uint8_t pic_set_irq_mask(uint16_t pic, uint8_t mask)
{
    outb(pic, mask);
}

void pic_eoi(int irq)
{
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    outb(PIC1_COMMAND, PIC_EOI);
}

uint8_t pic_read_isr(uint16_t pic)
{
    return inb(pic - 1);
}

bool pic_is_irq_spurious(int irq)
{
    if (irq == 7) {
        return !(pic_read_isr(PIC1) & 0x80);
    } else if (irq == 15) {
        return !(pic_read_isr(PIC2) & 0x80);
    }

    return false;
}
