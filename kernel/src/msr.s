global wrmsr
global rdmsr

wrmsr:
    mov rcx, rdi
    mov rax, rsi
    shr rsi, 32
    mov rdx, rsi
    wrmsr
    ret

rdmsr:
    mov rcx, rdi
    rdmsr
    shl rdx, 32
    or rax, rdx
    ret
