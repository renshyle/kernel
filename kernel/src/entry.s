global _start
extern kentry

section .bss

stack: resb 65536

section .text

_start:
    mov rsp, stack + 65536
    jmp kentry
