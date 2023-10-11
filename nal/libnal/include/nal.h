#ifndef NAL_H
#define NAL_H

#include <stdint.h>

#include "../../../kernel/src/syscalls.h"

typedef void (*signal_handler)(enum signal_type type);

uint64_t syscall(uint64_t syscallnum, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

void signal(enum signal_type type, uint64_t signal);
void sigsethandler(signal_handler handler);

#endif
