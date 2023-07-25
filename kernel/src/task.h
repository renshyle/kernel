#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stdnoreturn.h>

#include "virt.h"

#define TASK_TIME_SLICE_MS 20

struct __attribute__((__packed__)) tss {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint32_t reserved1[2];
    uint64_t ist[7];
    uint32_t reserved2[2];
    uint16_t reserved3;
    uint16_t iopb;
};

struct __attribute__((__packed__)) task {
    pml4e *page_map;

    // registers
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
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;

    uint64_t ticks;

    // llist
    struct task *next;
};

extern struct tss tss;
extern struct task *task_llist;
extern struct task *current_task;

void task_init(void);

int task_create(void *program, uint64_t size);

noreturn void task_switch(struct task *task);
noreturn void task_schedule(void);

void tss_load(void);

#endif
