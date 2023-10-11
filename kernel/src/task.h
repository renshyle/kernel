#ifndef TASK_H
#define TASK_H

#include <stddef.h>
#include <stdint.h>
#include <stdnoreturn.h>

#include "syscalls.h"
#include "virt.h"

#define TASK_TIME_SLICE_MS 20
#define SIGNAL_QUEUE_MIN_SIZE 64

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

struct task_cpu_state {
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
};

struct signal {
    enum signal_type type;
    union {
        uint64_t irq;
    } data;
    struct task *task; // NULL if signal is cancelled
};

struct task {
    pml4e *page_map;

    // registers
    struct task_cpu_state cpu_state;

    // scheduling
    uint64_t ticks;

    // signals
    uint64_t signal_entry;
    struct task_cpu_state signal_saved_state;
    uint64_t signal_stack;
    bool is_in_signal;
    struct signal current_signal;

    // llist
    struct task *next;
};

struct signal_queue {
    // a ring buffer with len elements starting at index i, signals signal->tesk == NULL are cancelled signals
    struct signal *signals;

    // the index of the next signal to pop
    size_t i;
    // the number of signals in the queue
    size_t len;
    // the size of the signals field in number of signals
    size_t size;
};;

extern struct tss tss;
extern struct task *task_llist;
extern struct task *current_task;

extern struct signal_queue signal_queue;

void task_init(void);

int task_create(void *program, uint64_t size);
void task_end(struct task *task);

struct signal signal_queue_pop(void);
void signal_queue_push(struct signal signal);

noreturn void task_dispatch_signal(struct signal signal);

noreturn void task_switch(struct task *task);
noreturn void task_schedule(void);

void tss_load(void);

#endif
