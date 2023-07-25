global _start
global kernel_stack
extern kentry

section .bss

resb 65536
kernel_stack:

section .text

_start:
    mov rsp, kernel_stack
    jmp kentry
