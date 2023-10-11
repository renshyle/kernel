// nal/libnal/include/syscalls.inc

#ifndef SYSCALLS_H
#define SYSCALLS_H

enum syscall {
    SYS_EXIT,
    SYS_SIGNAL,
    SYS_SET_SIGHANDLER,
    SYS_SIGRET,
};

enum signal_type {
    SIG_IRQ,
};

#endif
