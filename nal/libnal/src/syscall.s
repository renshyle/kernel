global syscall

syscall:
    mov r10, rcx
    syscall
    ret
