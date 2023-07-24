#ifndef PIC_H
#define PIC_H

#include <stdbool.h>
#include <stdint.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xa0
#define PIC2_DATA 0xa1
#define PIC1 PIC1_DATA
#define PIC2 PIC2_DATA

void pic_init(void);

uint8_t pic_get_irq_mask(uint16_t pic);
void pic_set_irq_mask(uint16_t pic, uint8_t mask);
uint8_t pic_read_isr(uint16_t pic);

bool pic_is_irq_spurious(int irq);
void pic_eoi(int irq);

#endif
