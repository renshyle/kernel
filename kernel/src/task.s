global tss_load
global task_user_switch

tss_load:
    mov ax, 0x28
    ltr ax
    ret

task_user_switch:
    add rsp, 16 ; skip return address and page_map
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx

    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    pop rax
    iretq
