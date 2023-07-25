#ifndef MSR_H
#define MSR_H

#include <stdint.h>

#define MSR_STAR 0xc0000081
#define MSR_LSTAR 0xc0000082
#define MSR_SFMASK 0xc0000085
#define MSR_IA32_EFER 0xc0000080

void wrmsr(uint32_t msr, uint64_t value);
uint64_t rdmsr(uint32_t msr);

#endif
