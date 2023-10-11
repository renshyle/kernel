global signal
global signal_entry
global sigsethandler

%include "syscalls.inc"

signal:
    mov rdx, rsi
    mov rsi, rdi
    mov rdi, SYS_SIGNAL
    syscall
    ret

sigsethandler:
    cmp qword [signal_handler], 0
    mov [signal_handler], rdi
    jne .entry_set
    ; the kernel's task structures don't have libnal's signal entry yet, give it using sys_set_sighandler
    mov rdi, SYS_SET_SIGHANDLER
    mov rsi, signal_entry
    syscall
    .entry_set:
    ; the kernel already knows about our signal entry, tell the kernel to enable te signal
    ret

signal_entry:
    mov rax, [signal_handler]
    test rax, rax
    jz .end
    call rax
    .end:
    mov rdi, SYS_SIGRET
    syscall

section .bss
signal_handler: resb 8
