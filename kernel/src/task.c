#include <stdbool.h>
#include <stdint.h>
#include <stdnoreturn.h>
#include <string.h>

#include "debug.h"
#include "panic.h"
#include "pit.h"
#include "phys.h"
#include "task.h"
#include "virt.h"

#define ELF_MAGIC 0x00010102464c457f
#define ELF_EXECUTABLE 2
#define ELF_AMD64 0x3e
#define ELF_VERSION 1

#define PT_LOAD 1

#define TASK_STACK_SIZE 8192
#define TASK_KERNEL_STACK_SIZE 32768

#define CHECK_OR_RET(comparison, error) \
if (!(comparison)) {\
    debug_write_string(error); \
    return -1; \
}

struct __attribute__((__packed__)) elf_header {
    uint64_t e_ident;
    uint8_t unused[8];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};

struct __attribute__((__packed__)) elf_program_header {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
};

struct tss tss;

struct task *task_llist;
struct task *current_task;

void task_init(void)
{
    tss.iopb = sizeof(struct tss); // iopb not used
    tss_load();
}

int task_create(void *program, uint64_t size)
{
    struct elf_header *header = program;

    CHECK_OR_RET(size >= sizeof(struct elf_header), "too small to be elf\n")
    CHECK_OR_RET(header->e_ident == ELF_MAGIC, "invalid elf magic\n")
    CHECK_OR_RET(header->e_type == ELF_EXECUTABLE, "not an executable\n")
    CHECK_OR_RET(header->e_version == ELF_VERSION, "wrong elf version\n")
    CHECK_OR_RET(header->e_machine == ELF_AMD64, "wrong architecture\n")
    CHECK_OR_RET(header->e_phentsize == sizeof(struct elf_program_header), "wrong program header size\n")
    CHECK_OR_RET(size >= header->e_phoff + header->e_phnum * sizeof(struct elf_program_header), "program headers missing\n")

    struct task *task = PHYSICAL_TO_VIRTUAL(phys_alloc_page()); // wastes 4 kb - sizeof(struct task) memory
    memset(task, 0, PAGE_SIZE);

    pml4e *page_map = virt_create_page_map();

    task->page_map = page_map;

    struct elf_program_header *program_headers = (struct elf_program_header*) ((char*) program + header->e_phoff);

    for (uint64_t i = 0; i < header->e_phnum; i++) {
        struct elf_program_header program_header = program_headers[i];

        if (program_header.p_type != PT_LOAD) {
            continue;
        }

        if (program_header.p_memsz < program_header.p_filesz) {
            debug_write_string("memsz less than filesz, cannot continue\n");
            // FIXME: cleanup
            return -1;
        }

        if (size < program_header.p_filesz + program_header.p_offset) {
            debug_write_string("program too small\n");
            // FIXME: cleanup
            return -1;
        }

        if (program_header.p_align != PAGE_SIZE || (program_header.p_vaddr & (PAGE_SIZE - 1)) != 0) {
            debug_write_string("segment alignment wrong\n");
            // FIXME: cleanup
            // TODO: support arbitrary alignment
            return -1;
        }

        uint64_t req_pages = (program_header.p_memsz + PAGE_SIZE - 1) / PAGE_SIZE;
        uint64_t to_write = program_header.p_filesz;
        for (uint64_t j = 0; j < req_pages; j++) {
            uint64_t page = phys_alloc_page();

            if (page == 0) {
                panic("out of memory");
            }

            if (to_write > PAGE_SIZE) {
                memcpy(PHYSICAL_TO_VIRTUAL(page), (char*) program + program_header.p_offset, PAGE_SIZE); // assumes pages returned by phys_alloc_page are identity mapped
                to_write -= PAGE_SIZE;
            } else if (to_write > 0) {
                memcpy(PHYSICAL_TO_VIRTUAL(page), (char*) program + program_header.p_offset + j * PAGE_SIZE, to_write); // assumes pages returned by phys_alloc_page are identity mapped
                memset(PHYSICAL_TO_VIRTUAL(page + to_write), 0, PAGE_SIZE - to_write);

                to_write = 0;
            } else {
                memset(PHYSICAL_TO_VIRTUAL(page), 0, PAGE_SIZE);
            }

            virt_map(page_map, program_header.p_vaddr, page, true);
        }
    }

    // allocate task stack

    uint64_t stack_address = virt_find_free_area(page_map, TASK_STACK_SIZE / PAGE_SIZE, true);

    if (stack_address == 0) {
        panic("out of memory");
    }

    for (int i = 0; i < TASK_STACK_SIZE / PAGE_SIZE; i++) {
        uint64_t page = phys_alloc_page();

        if (page == 0) {
            panic("out of memory");
        }

        virt_map(page_map, stack_address + i * PAGE_SIZE, page, true);
    }

    // allocate kernel stack

    uint64_t kernel_stack_address = virt_find_free_area(page_map, TASK_KERNEL_STACK_SIZE / PAGE_SIZE, false);

    if (kernel_stack_address == 0) {
        panic("out of memory");
    }

    for (int i = 0; i < TASK_KERNEL_STACK_SIZE / PAGE_SIZE; i++) {
        uint64_t page = phys_alloc_page();

        if (page == 0) {
            panic("out of memory");
        }

        virt_map(page_map, kernel_stack_address + i * PAGE_SIZE, page, false);
    }

    task->rip = header->e_entry;
    task->cs = 0x20 | 3;
    task->ss = 0x18 | 3;
    task->rflags = 0x202; // interrupts enabled
    task->rsp = stack_address + TASK_STACK_SIZE;
    task->kernel_stack = kernel_stack_address + TASK_KERNEL_STACK_SIZE;

    if (task_llist == NULL) {
        task_llist = task;
    } else {
        task->next = task_llist;
        task_llist = task;
    }

    return 0;
}

// task_user_switch should only be called by task_switch
extern noreturn void task_user_switch(struct task task);

noreturn void task_switch(struct task *task)
{
    set_cr3(kvirtual_to_physical((uint64_t) task->page_map));

    tss.rsp0 = task->kernel_stack;

    task->ticks = (PIT_FREQUENCY_HZ * TASK_TIME_SLICE_MS) / 1000;

    current_task = task;

    task_user_switch(*task);
}

noreturn void task_schedule(void)
{
    struct task *task = NULL;

    if (current_task == NULL) {
        if (task_llist != NULL) {
            task = task_llist;
        }
    } else {
        if (current_task->next == NULL) {
            task = task_llist;
        } else {
            task = current_task->next;
        }
    }

    if (task == NULL) {
        panic("no tasks running");
    }

    task_switch(task);
}
