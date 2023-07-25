[bits 64]

global syscall_entry
extern syscall

extern kernel_stack

STRUCT_TASK_KERNEL_STACK_OFFSET equ 168

; the syscall calling convention is similar to the system v amd64 calling convention:
; parameters are passed in rdi, rsi, rdx, r10, r8, r9 (in that order) (note that r10 is used instead of rcx)
; no parameters are passed on the stack
; the first parameter (rdi) is the syscall number
;
; the return value is returned in rax
;
; register preservation is the same as in the system v amd64 calling convention:
; rbx, rsp, rbp, and r12-r15 are preserved
syscall_entry:
    ; set up the kernel stack
    mov rax, rsp
    mov rsp, kernel_stack
    push rax
    push r11
    push rcx
    push rbp

    ; parameters to syscall()
    push r9
    push r8
    push r10
    push rdx
    push rsi
    mov rsi, rsp

    cld
    xor rbp, rbp
    call syscall
    add rsp, 40

    ; clear non-preserved registers
    xor rdi, rdi
    xor rsi, rsi
    xor rdx, rdx
    xor r8, r8
    xor r9, r9
    xor r10, r10

    pop rbp
    pop rcx
    pop r11
    mov rsp, [rsp]
    o64 sysret
