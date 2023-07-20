#include "debug.h"

void panic(char *msg)
{
    __asm__("cli");

    debug_write_string("panic panic panic PANIC PANIC PANIC: ");
    debug_write_string(msg);
    debug_write_string("\n");

    while (1) {
        __asm__("hlt");
    }
}
