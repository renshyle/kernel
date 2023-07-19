#include <stdint.h>

#include "debug.h"
#include "int.h"

#define IDT_DEFAULT_GATE(i) idt_set_gate(i, (uint64_t) interrupt_handler_ ## i, 0x28, IDT_ATTR_PRESENT | IDT_ATTR_INTERRUPT_GATE);

struct idtr idtr;

struct idt_entry idt_entries[256];

void int_init(void)
{
    IDT_DEFAULT_GATE(0)
    IDT_DEFAULT_GATE(1)
    IDT_DEFAULT_GATE(2)
    IDT_DEFAULT_GATE(3)
    IDT_DEFAULT_GATE(4)
    IDT_DEFAULT_GATE(5)
    IDT_DEFAULT_GATE(6)
    IDT_DEFAULT_GATE(7)
    IDT_DEFAULT_GATE(8)
    IDT_DEFAULT_GATE(10)
    IDT_DEFAULT_GATE(11)
    IDT_DEFAULT_GATE(12)
    IDT_DEFAULT_GATE(13)
    IDT_DEFAULT_GATE(14)
    IDT_DEFAULT_GATE(16)
    IDT_DEFAULT_GATE(17)
    IDT_DEFAULT_GATE(18)

    idtr.offset = (uint64_t) idt_entries;
    idtr.size = 256 * sizeof(struct idt_entry) - 1;

    idt_load();
    debug_write_string("idt loaded\n");
}

void idt_set_gate(int i, uint64_t handler, uint16_t cs, uint16_t attr)
{
    idt_entries[i].offset_low = (uint16_t) (handler & 0xffff);
    idt_entries[i].offset_mid = (uint16_t) (handler >> 16) & 0xffff;
    idt_entries[i].offset_high = (uint32_t) (handler >> 32) & 0xffffffff;
    idt_entries[i].attr = attr;
    idt_entries[i].reserved = 0;
    idt_entries[i].cs = cs;
}

void interrupt(struct interrupt_frame frame)
{
    debug_write_string("interrupt 0x");
    debug_write_uint64(frame.interrupt);
    debug_write_string("\n");
}
