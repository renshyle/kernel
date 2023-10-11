#include <stdint.h>
#include <string.h>

#include "debug.h"
#include "int.h"
#include "msr.h"
#include "pic.h"
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

    switch (syscallnum) {
        case SYS_EXIT:
            task_end(current_task);
            task_schedule();
            break;
        case SYS_SET_SIGHANDLER:
            current_task->signal_entry = parameters[0];
            break;
        case SYS_SIGNAL:
            if (current_task->signal_entry == 0) {
                debug_write_string("task tried to listen for a signal without a signal handler!\n");
                return 1;
            }

            switch (parameters[0]) {
                case SIG_IRQ:;
                    uint64_t irq = parameters[1];

                    if (irq >= MAX_IRQS) {
                        debug_write_string("task tried to listen for an irq that doesn't exist!\n");
                        return 1;
                    }

                    if (irq_handlers[irq] != NULL) {
                        debug_write_string("task tried to listen for an irq that is already being listened to!\n");
                        return 1;
                    }

                    irq_handlers[irq] = current_task;

                    break;
                default:
                    debug_write_string("unknown signal type\n");
                    return 1;
            }
            break;
        case SYS_SIGRET:
            if (!current_task->is_in_signal) {
                return 1;
            }

            memcpy(&current_task->cpu_state, &current_task->signal_saved_state, sizeof(struct task_cpu_state));
            current_task->is_in_signal = false;

            if (current_task->current_signal.type == SIG_IRQ) {
                pic_eoi(current_task->current_signal.data.irq);
            }

            task_schedule();
            break;
        default:
            debug_write_string("unknown syscall\n");
            task_end(current_task);
            task_schedule();
            break;
    }

    return 0;
}
