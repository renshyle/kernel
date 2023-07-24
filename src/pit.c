#include "io.h"
#include "pit.h"

#define PIT_CHANNEL0_DATA 0x40
#define PIT_COMMAND 0x43

#define PIT_CHANNEL0 0
#define PIT_LOHI_ACCESS 0x30
#define PIT_MODE3 0x06

void pit_init(void)
{
    outb(PIT_COMMAND, PIT_CHANNEL0 | PIT_LOHI_ACCESS | PIT_MODE3);

    int divisor = PIT_NATURAL_FREQUENCY_HZ / PIT_FREQUENCY_HZ;
    outb(PIT_CHANNEL0_DATA, divisor & 0xff);
    outb(PIT_CHANNEL0_DATA, (divisor >> 8) & 0xff);
}
