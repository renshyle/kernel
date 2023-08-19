#include "debug.h"
#include "msr.h"
#include "syscall.h"
#include "task.h"

void syscall_init(void)
{
    wrmsr(MSR_STAR, 0x0013000800000000); // sysret cs/ss is 0x13, syscall cs/ss is 0x08
    wrmsr(MSR_LSTAR, (uint64_t) syscall_entry);
    wrmsr(MSR_SFMASK, 0x200); // clear interrupt flag

    uint64_t efer = rdmsr(MSR_IA32_EFER);
    wrmsr(MSR_IA32_EFER, efer | 1); // enable the syscall instruction
}

uint64_t syscall(enum syscall syscallnum, uint64_t parameters[5])
{
    debug_write_string("syscall 0x");
    debug_write_uint64(syscallnum);
    debug_write_string("\nparameters:\n");

    for (int i = 0; i < 5; i++) {
        debug_write_string("  0x");
        debug_write_uint64(parameters[i]);
        debug_write_string("\n");
    }

    if (syscallnum == SYS_EXIT) {
        task_end(current_task);
        task_schedule();
    }

    return 0;
}
