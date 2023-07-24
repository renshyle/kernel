#include "debug.h"

extern uint64_t get_return_address(uint64_t rbp);
extern uint64_t get_previous_rbp(uint64_t rbp);

void panic(char *msg)
{
    __asm__("cli");

    debug_write_string("panic panic panic PANIC PANIC PANIC: ");
    debug_write_string(msg);
    debug_write_string("\n");

    // stack trace
    debug_write_string("stack trace:\n");
    
    uint64_t rbp = get_previous_rbp(0);
    while (rbp != 0) {
        debug_write_string("0x");
        debug_write_uint64(get_return_address(rbp));
        debug_write_string("\n");

        rbp = get_previous_rbp(rbp);
    }

    while (1) {
        __asm__("hlt");
    }
}
