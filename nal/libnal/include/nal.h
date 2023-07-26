#ifndef NAL_H
#define NAL_H

#include <stdint.h>

#define NAL_SYS_EXIT 0

uint64_t syscall(uint64_t syscallnum, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

#endif
