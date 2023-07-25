extern main
global _start

section .data
argv: dq 0

section .text

_start:
    ; rdi (argc) is already 0
    mov rsi, argv
    call main
    jmp $
