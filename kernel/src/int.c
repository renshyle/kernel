#include <stdint.h>

#include "debug.h"
#include "int.h"
#include "panic.h"
#include "pic.h"
#include "task.h"

#define IRQ_PIT 0

#define IDT_DEFAULT_GATE(i) idt_set_gate(i, (uint64_t) interrupt_handler_ ## i, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INTERRUPT_GATE);

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

    // irq
    IDT_DEFAULT_GATE(32)
    IDT_DEFAULT_GATE(33)
    IDT_DEFAULT_GATE(34)
    IDT_DEFAULT_GATE(35)
    IDT_DEFAULT_GATE(36)
    IDT_DEFAULT_GATE(37)
    IDT_DEFAULT_GATE(38)
    IDT_DEFAULT_GATE(39)
    IDT_DEFAULT_GATE(40)
    IDT_DEFAULT_GATE(41)
    IDT_DEFAULT_GATE(42)
    IDT_DEFAULT_GATE(43)
    IDT_DEFAULT_GATE(44)
    IDT_DEFAULT_GATE(45)
    IDT_DEFAULT_GATE(46)
    IDT_DEFAULT_GATE(47)

    idtr.offset = (uint64_t) idt_entries;
    idtr.size = 256 * sizeof(struct idt_entry) - 1;

    idt_load();
    __asm__("sti");
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
    if (frame.interrupt >= 32 && frame.interrupt <= 47) {
        // irq
        uint64_t irq = frame.interrupt - 32;

        if (pic_is_irq_spurious(irq)) {
            debug_write_string("spurious interrupt 0x");
            debug_write_uint64(irq);
            debug_write_string("\n");
            return;
        }

        if (irq == IRQ_PIT) {
            if ((frame.cs & 3) == 3) {
                // pit irq happened while in userspace, meaning we can save the task state and schedule the next task
                struct task *task = current_task;

                if (task->ticks == 0) {
                    task->r15 = frame.r15;
                    task->r14 = frame.r14;
                    task->r13 = frame.r13;
                    task->r12 = frame.r12;
                    task->r11 = frame.r11;
                    task->r10 = frame.r10;
                    task->r9 = frame.r9;
                    task->r8 = frame.r8;
                    task->rbp = frame.rbp;
                    task->rdi = frame.rdi;
                    task->rsi = frame.rsi;
                    task->rdx = frame.rdx;
                    task->rcx = frame.rcx;
                    task->rbx = frame.rbx;
                    task->rax = frame.rax;
                    task->rip = frame.rip;
                    task->rflags = frame.rflags;
                    task->rsp = frame.rsp;

                    // i guess you can technically change these from userspace so might as well save them
                    // the other segment selectors are forced to 0x23, though
                    task->ss = frame.ss;
                    task->cs = frame.cs;

                    pic_eoi(irq);
                    task_schedule();
                }
            }
        }

        pic_eoi(irq);
    } else {
        debug_write_string("interrupt 0x");
        debug_write_uint64(frame.interrupt);
        debug_write_string("\nerror code: 0x");
        debug_write_uint64(frame.error_code);
        debug_write_string("\nrip: 0x");
        debug_write_uint64(frame.rip);
        debug_write_string("\n");

        panic("unexpected interrupt");
    }
}
