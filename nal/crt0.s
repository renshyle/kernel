extern main
global _start

SYS_EXIT equ 0

section .data
argv: dq 0

section .text

_start:
    ; rdi (argc) is already 0
    mov rsi, argv
    call main
    ; exit
    mov rdi, SYS_EXIT
    syscall
