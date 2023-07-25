#include <stdint.h>
#include <stdlib.h>
#include <limine.h>

#include "debug.h"
#include "gdt.h"
#include "int.h"
#include "panic.h"
#include "phys.h"
#include "pit.h"
#include "pic.h"
#include "syscall.h"
#include "task.h"
#include "virt.h"

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static volatile struct limine_bootloader_info_request info_request = {
    .id = LIMINE_BOOTLOADER_INFO_REQUEST,
    .revision = 0
};

static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

#define LIMINE_ADDR_TO_VIRTUAL(x) ((void*) ((uint64_t) (x) - limine_hhdm_offset + PHYSICAL_MEMORY))
void kentry(void)
{
    uint64_t limine_hhdm_offset = hhdm_request.response->offset;
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    uint32_t *fb_ptr = framebuffer->address;
    uint64_t pitch = framebuffer->pitch;
    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    for (size_t i = 0; i < 100; i++) {
        fb_ptr[i * (pitch / 4) + i] = 0xffffff;
    }

    debug_init();
    debug_write_string("booted with ");
    debug_write_string(info_request.response->name);
    debug_write_string(" ");
    debug_write_string(info_request.response->version);
    debug_write_string(", information revision 0x");
    debug_write_uint64(info_request.response->revision);
    debug_write_byte('\n');

    debug_write_string("loaded at physical address 0x");
    debug_write_uint64(kernel_address_request.response->physical_base);
    debug_write_string("\nvirtual address 0x");
    debug_write_uint64(kernel_address_request.response->virtual_base);
    debug_write_string("\n");

    gdt_init();
    phys_init(memmap_request.response);
    pit_init();
    int_init();
    pic_init();
    task_init();
    syscall_init();
    virt_init(kernel_address_request.response->physical_base);
    // from this point all physical addresses should be accessed with PHYSICAL_TO_VIRTUAL()
    // and limine provided memory should be accessesd using LIMINE_ADDR_TO_VIRTUAL()

    // load modules
    if (module_request.response == NULL) {
        panic("no modules");
    }

    struct limine_module_response *module_response = LIMINE_ADDR_TO_VIRTUAL(module_request.response);
    debug_write_string("found 0x");
    debug_write_uint64(module_response->module_count);
    debug_write_string(" modules\n");

    debug_write_string("modules:\n");
    struct limine_file **modules = LIMINE_ADDR_TO_VIRTUAL(module_response->modules);
    for (uint64_t i = 0; i < module_response->module_count; i++) {
        struct limine_file *module = LIMINE_ADDR_TO_VIRTUAL(modules[i]);
        debug_write_string("  ");
        debug_write_string(LIMINE_ADDR_TO_VIRTUAL(module->path));
        debug_write_string("\n");

        int r = task_create(LIMINE_ADDR_TO_VIRTUAL(module->address), module->size);

        if (r < 0) {
            debug_write_string("failed to load module\n");
        }
    }

    task_schedule();
}
