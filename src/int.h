#ifndef INT_H
#define INT_H

#include <stdint.h>

#define IDT_ATTR_INTERRUPT_GATE 0x0e00
#define IDT_ATTR_PRESENT 0x8000

struct __attribute__((__packed__)) interrupt_frame {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t interrupt;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

struct __attribute__((__packed__)) idtr {
    uint16_t size;
    uint64_t offset;
};

struct __attribute__((__packed__)) idt_entry {
    uint16_t offset_low;
    uint16_t cs;
    uint16_t attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
};

void int_init(void);

void interrupt(struct interrupt_frame frame);

void idt_set_gate(int i, uint64_t handler, uint16_t cs, uint16_t attr);
extern void idt_load(void);

extern void interrupt_handler_0();
extern void interrupt_handler_1();
extern void interrupt_handler_2();
extern void interrupt_handler_3();
extern void interrupt_handler_4();
extern void interrupt_handler_5();
extern void interrupt_handler_6();
extern void interrupt_handler_7();
extern void interrupt_handler_8();
extern void interrupt_handler_10();
extern void interrupt_handler_11();
extern void interrupt_handler_12();
extern void interrupt_handler_13();
extern void interrupt_handler_14();
extern void interrupt_handler_16();
extern void interrupt_handler_17();
extern void interrupt_handler_18();

#endif
